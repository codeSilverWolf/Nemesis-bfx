

from pathlib import Path, PurePath, PurePosixPath, WindowsPath
import os
import shutil
import subprocess
import re
import argparse
import json

def getMsysRootDir():
    """
    returns string containing windows compatible path to root directory '/' for msys. usually c:\\msys64\\
    returns None if cygpath.exe program is not available
    """
    subproc = subprocess.run(["cygpath","-w","/"],capture_output=True, text=True)
    if subproc:
        lines = subproc.stdout.splitlines()
        if len(lines) > 0:
            path = Path(lines[0])
            return path
    return None


class MakeModConfig:
    def __init__(self) -> None:
        self.nemesis_program_name = "NemesisUnlimitedBehaviorEngine.exe"        # compiled executable name
        self.nemesis_program_name_in_mod = "Nemesis Unlimited Behavior Engine.exe"        # compiled executable name
        self.work_dir = Path.cwd()      # current working directory, should point to path to build dir containing compiled executable
        self.mod_root_dir = self.work_dir / "Nemesis-bfx/Data/Nemesis_Engine"
        self.msys_root_dir_win = Path("c:/msys64") # default , should be overwritten
        if "MSYSTEM" in os.environ :
            self.running_in_msys = True     # are we running inside msys2 shell?
            self.msys = os.environ["MSYSTEM"]
            self.msys_prefix = os.environ["MSYSTEM_PREFIX"]
            self.msys_root_dir_win = getMsysRootDir()   # get root path for current msys environment (windows compatible path)
        else:
            self.running_in_msys = False
        self.include_licenses = False   # set to True if we should include licenses for used msys packages (dlls mainly)

# global config, instance of class MakeModConfig
g_config = MakeModConfig()


def msysPathToWindows(path):
    """
    translate msys unix path to corresponding windows path
    by adding msys root prefix path in windows style before it
    """
    global g_config

    if not isinstance(path, Path):
        path = Path(path)
    
    if g_config.msys_root_dir_win != None:
        return g_config.msys_root_dir_win / (path.relative_to("/"))
    else:
        return None
    

def getDllsFromExecutable(config: MakeModConfig, filepath):
    dlls = { "SYSTEM" : [], "MSYS": []}
    run_args = ["ldd", filepath]
    # run ldd.exe and capture output for processing
    cp = subprocess.run(run_args, capture_output=True, text=True)
    if cp.returncode == 0:
        dll_lines = cp.stdout.splitlines()
        pattern = re.compile(r'^\s*([\S]+)\s=>\s([\S]+)\s')
        pattern_systemdll = re.compile(r'/[a-zA-Z]/windows/system32/.+\.dll', flags=re.IGNORECASE)
        pattern_msysdll = re.compile(r'/'+config.msys+ r'/bin/.+\.dll', flags=re.IGNORECASE)
        for line in dll_lines:
            dll_match = pattern.match(line)
            if dll_match:
                dll_name = dll_match.group(1)
                dll_full_path = dll_match.group(2)
                if pattern_systemdll.match(dll_full_path):
                    # windows dll - do not copy to mod
                    dlls["SYSTEM"].append(PurePosixPath(dll_full_path))
                elif pattern_msysdll.match(dll_full_path):
                    #  dll from msys , we need this in same directory as executable
                    dlls["MSYS"].append(PurePosixPath(dll_full_path))
                else:
                    print("error: unable to match dll to category. dll=[",dll_full_path,']',sep='')

        return dlls
    else:
        return None

def getPackageInfoFromPacman(package_name: str):
    pkg_info = {}
    # use 'pacman -Si' to find package informations
    subproc = subprocess.run(["pacman","-Qi",package_name],capture_output=True, text=True)
    if subproc:
        #print("return code ",subproc.returncode)
        if subproc.returncode == 130:
            print("subprocess was interrupted!")
            return None
        lines = subproc.stdout.splitlines()
        if len(lines) > 0:
            for line in lines:
                (field_name, tmp_sep, field_text) = line.partition(":")
                field_name = field_name.strip()
                field_text = field_text.strip()

                pkg_info[field_name] = field_text
                # print(line)

    return pkg_info
         

def getFileOwningPackageFromPacman(filename):
    print("searching for file", filename, "in pacman database")

    filename = str(filename) # change to str - serialize to json fails with Path type objects
    owning_package = {}
    # use pacman -F to find package owning current file
    subproc = subprocess.run(["pacman","-F","--quiet",filename],capture_output=True, text=True)
    if subproc:
        #print("return code ",subproc.returncode)
        if subproc.returncode == 130:
            print("subprocess was interrupted!")
            return None
        lines = subproc.stdout.splitlines()
        if len(lines) > 0:
            owning_package_arr = lines[0].split("/")
            if(len(owning_package_arr) == 2):
                owning_package['NAME'] = owning_package_arr[1]
                owning_package['SUBSYSTEM'] = owning_package_arr[0]
                print("  found owning package ", owning_package_arr[1], "for subsystem", owning_package_arr[0])
            else:
                owning_package['NAME'] = owning_package_arr[0]
                owning_package['SUBSYSTEM'] = ""
                print("  warning: found package but it has only name",owning_package['NAME'])

            # get package 'root' name - without subsystem and compiler prefix 
            if owning_package['SUBSYSTEM'] == "ucrt64":
                owning_package['SHORTNAME'] = owning_package['NAME'].removeprefix("mingw-w64-ucrt-x86_64-")
        owning_package['FILES'] = [filename]
    else:
        # print("Can't find package owning file ",filename,"!!!")
        owning_package = None
    return owning_package

def getMsysPackageInfoForFiles(filenames):
    """
        This function uses pacman to get info about packages containing specified files.

        filenames : array of PurePath or derived classes containing file names to search for
    """
    packages = {}
    cache_available = False

    cache_file = Path("makemod_pkg_cache.json")
    if cache_file.is_file():
        with cache_file.open("r", encoding="utf-8") as cf:
            packages = json.load(cf)
            cache_available = True

    for filename in filenames:
        # try to get file from already cached packages
        file_was_in_cache = False
        if cache_available:
            print("searching cache for", filename)
            for pkg_name in packages:
                if 'FILES' in packages[pkg_name] and str(filename) in packages[pkg_name]['FILES']:
                    print("  file ", str(filename), "was found in cache in pkg", pkg_name)
                    file_was_in_cache = True
                    break

        if file_was_in_cache: # pkg owning file is in cache already, continue loop with next file
            continue

        # pkg owning file not in cache. Search Pacman database
        if owning_package := getFileOwningPackageFromPacman(filename):
            pkg_name = owning_package['NAME']
            if pkg_name not in packages:
                packages[pkg_name] = owning_package
                # get new package info
                packages[pkg_name]['INFO'] = getPackageInfoFromPacman(pkg_name)
            else:
                # only add another file to already discovered package
                packages[pkg_name]['FILES'].append(str(filename))   # convert filename (class Path) to str or json serializer will fail
        else:
            print("Can't find package owning file ",filename,"!!!")
    
    # dump used packages for caching
    with cache_file.open("w", encoding="utf-8") as cf:
        json.dump(packages, cf, sort_keys=True, indent=2)
    
    return packages

def makeLicensesDir(config: MakeModConfig, packages):
    mod_lic_dir = Path("Licenses")  # Directory for 3rd party license files in created mod
    lic_file_name = config.mod_root_dir / mod_lic_dir / Path("Information about used 3rd party libraries.txt")

    if not lic_file_name.parent.exists():
       lic_file_name.parent.mkdir(parents=True)

    lic_file = lic_file_name.open("w", encoding="utf-8")

    lic_file.write("""
This file lists information about open source libraries that Nemesis-bfx links dynamically to.
         
"""
    )

    msys_lic_dir = config.msys_root_dir_win / config.msys_prefix / "share/licenses"
    print(f"Copying license directories from: {str(msys_lic_dir)}")

    for pkg_name, pkg in packages.items():
        info_lines = []
        info_lines.append(f"msys2 package name: {pkg_name}\n")
        info_lines.append(f"Description: {pkg['INFO']['Description']}\n")
        info_lines.append(f"license(s): {pkg['INFO']['Licenses']}\n")
        info_lines.append(f"homepage: {pkg['INFO']['URL']}\n")
        used_files = [str(PurePath(uf).name) for uf in pkg['FILES']]
        info_lines.append("Used libraries: " + ", ".join(used_files) + "\n")
        info_lines.append("\n")
        lic_file.writelines(info_lines)

        # copy files from directories in msys containing package licenses to mod
        print(f"for package: {pkg['SHORTNAME']}")
        current_lic_dir = msys_lic_dir / pkg['SHORTNAME']
        if current_lic_dir.is_dir():
            for f in current_lic_dir.iterdir():
                # f is full path
                if f.is_file():
                    copyToMod(config, f, mod_lic_dir / pkg['SHORTNAME'])
        else:
            print(f"INFO: for package {pkg_name} directory with licenses does not exist in msys!")
    
    lic_file.close()




def copyToMod(config: MakeModConfig, src_path: Path, dst_path: Path = Path("."), create_dirs = True, dst_path_is_file=False) -> bool:
    """
    copy file to directory _relative_ to mod root path, overwriting destination files when necessary
    
    src_path: can be Path, or _absolute_ PurePosixPath - represents path inside current msys environment
            (usefull for copying dlls etc. from MSYS2).
    dst_path: relative path pointing to destination directory or
              path to destination filename (when argument dst_path_is_file == True)
    create_dirs == True: create missing destination directories
    """
    
    # convert unix path to windows
    if type(src_path) is PurePosixPath:
        if src_path.root == "/":
            src_path = config.msys_root_dir_win / src_path.relative_to('/')
    
    #convert dst path to Path from str if needed
    if type(dst_path) is str:
        dst_path = Path(dst_path)
    
    if not src_path.is_file():
        print("ERROR: cannot copy - src path", str(src_path), "is not a file or does not exists!")
        return False
    if dst_path.root or dst_path.drive:
        print("ERROR: cannot copy - file ", str(src_path), " Destination path '", dst_path,"' has to be relative!")
    
    dst_path = config.mod_root_dir / dst_path

    # get destination directory from dst_path
    if dst_path_is_file:
        dst_dir = dst_path.parent # destination path represents file, so we need to get parent directory
    else:
        dst_dir = dst_path
    
    # create destination directory if needed and requested. Return error otherwise.
    if not dst_dir.exists():
        if create_dirs:
            dst_dir.mkdir(parents=True)
            print("making path:", str(dst_dir))
        else:
            print("ERROR: destinition directory ", str(dst_dir), "does not exists!")
            return False
    
    # dst_file is full file path to be copied to
    if dst_path_is_file:
        dst_file = dst_path
    else: 
        # dst_path is directory so add fsrc file name
        dst_file = dst_path / src_path.name

    file_was_overwritten = False # flag that signalizes if the destination file was older and overwritten
    skip_file = False       # if dst file is same/newer - skip it

    # check if destination file exists and delete if older/different
    if dst_file.exists():
        src_stat = src_path.stat()
        dst_file_stat = dst_file.stat()
        if src_stat.st_mtime > dst_file_stat.st_mtime or src_stat.st_size != dst_file_stat.st_size:
            dst_file.unlink()
            file_was_overwritten = True
        else:
            skip_file = True
    
    # finally we can copy
    if skip_file:
        print("SKIP ",str(src_path), "->", str(dst_file) )
    else:
        rv = shutil.copy2(src_path, dst_file)
        print("OVERWRITE" if file_was_overwritten else "COPY", str(src_path), "->", str(rv))

    return True
    

def makePythonLibrary(config: MakeModConfig):
    #exclude diretories from python library
    python_lib_exclude = ["distutils", "ensurepip", "lib2to3", "pydoc_data", "test", "turtledemo"]

    python_interpreter_path = "/ucrt64/bin/python3.exe"
    pacman_pkg_cache_dir = PurePosixPath("/var/cache/pacman/pkg/")
    python_pkg_archive_posix = PurePosixPath()
    python_pkg_subsystem = ""
    python_ver_major:int = 0
    python_ver_minor:int = 0

    python_found = False

    # zip python library
    mod_python_libs_dir = config.mod_root_dir / Path("python_libs")  # directory where archive with python libraries should be copied to
    if not mod_python_libs_dir.exists():
        mod_python_libs_dir.mkdir(parents=True)
    elif not mod_python_libs_dir.is_dir():
        print("ERROR: path for python libs exists and is not directory!!! aborting ")
        return False

    print("Searching for python interpreter...")
    python_pkg = getMsysPackageInfoForFiles( [ PurePath(python_interpreter_path) ])
    if python_pkg != None :
        for pkgname, pkg in python_pkg.items():
            if "python" in pkgname and python_interpreter_path in pkg['FILES']:
                print(f"found python package : {pkgname} version {pkg['INFO']['Version']}")
                python_found = True
                python_pkg_archive_posix = pacman_pkg_cache_dir / (f"{pkgname}-{pkg['INFO']['Version']}-any.pkg.tar.zst")
                ver_list = pkg['INFO']['Version'].split(".")
                python_ver_major = int(ver_list[0])
                python_ver_minor = int(ver_list[1])
                python_pkg_subsystem = pkg['SUBSYSTEM']
    
    if not python_found:
        print("ERROR: python package not found. Aborting!!!")
        exit(1)

    #python_libs_zip_output = mod_python_libs_dir / Path(f"python{python_ver_major}{python_ver_minor}")  # zip file extension will be added automaticly by shutil.make_archive
    py_pkg_path = config.msys_root_dir_win / Path(python_pkg_archive_posix.relative_to("/"))  # windows path to python pkg archive
    print("Making python libs zip archive")
    print("from package:", str(py_pkg_path))

    #check if python zip need updating. Condition: created archive older than curent python package from msys
    dst_file = mod_python_libs_dir / Path(f"python{python_ver_major}{python_ver_minor}.zip")
    py_pkg_path = config.msys_root_dir_win / Path(python_pkg_archive_posix.relative_to("/"))
    create_python_archive = True
    if dst_file.exists() :
                src_stat = py_pkg_path.stat()
                dst_file_stat = dst_file.stat()
                if src_stat.st_mtime > dst_file_stat.st_mtime :
                    dst_file.unlink()
                else:
                    create_python_archive = False

    if create_python_archive:
        tmp_dir = Path("makemod_tmp")
        # tmp_package_name = python_pkg_archive_posix.stem
        python_tmp_libs_dir = tmp_dir / "python_libs"  # path to directory containing extracted pkg file 

        if python_tmp_libs_dir.is_dir() :
            print("Cleaning tmp directory")
            shutil.rmtree(python_tmp_libs_dir)

        python_tmp_libs_dir.mkdir(parents=True)

        #extract archive , use bsdtar becouse normal tar does not work
        command = ["bsdtar","-x","-f",py_pkg_path,"-C",python_tmp_libs_dir,"--strip-components","3"]
        for excl_path in python_lib_exclude:
            command.append("--exclude")
            command.append(f"{python_pkg_subsystem}/lib/python{python_ver_major}.{python_ver_minor}/{excl_path}")
        command.append(f"{python_pkg_subsystem}/lib/python{python_ver_major}.{python_ver_minor}")  # extract only library files
        print("DBG: tar command = ", command)

        print("Unpacking package...")
        subproc = subprocess.run(command,capture_output=False, text=False)
        if subproc:
            if subproc.returncode == 0:
                print("Unpacking package complete")
            else:
                print("ERROR tar failed with return code ",subproc.returncode)
                return None
            #lines = subproc.stdout.splitlines()

        print("zipping python libs...")
        shutil.make_archive(str(dst_file.with_suffix("")), 'zip', str(python_tmp_libs_dir))
        
        # copy "lib-dynload" directory from python lib without zipping
        # without these files embedded python can not open zip archive with library files
        system_dynlib_path = python_tmp_libs_dir / Path("lib-dynload")
        with os.scandir(system_dynlib_path) as dir_it:
            for entry in dir_it:
                if entry.is_file():
                    copyToMod(config, system_dynlib_path / entry.name, Path(Path("python_libs/lib-dynload")))
    else:
        print("Python libs archive", str(dst_file), "is up to date. Skipping copying lib-dynload directory too!")


def makeMod(config: MakeModConfig):

    nem_path = Path(config.nemesis_program_name)
    if not nem_path.is_file:
        print("ERROR: Nemesis executable '",nem_path,"'not found. Current work directory:", Path.cwd())
        return None
    print("Found menesis executable. Building mod directory.")
    dlls = getDllsFromExecutable(config, config.nemesis_program_name)

    if not config.mod_root_dir.exists():
        config.mod_root_dir.mkdir(parents=True)

    # get licenses
    if config.include_licenses and dlls:
        pkgs_for_libraries = getMsysPackageInfoForFiles(dlls['MSYS'])
        makeLicensesDir(config, pkgs_for_libraries)

    # copy files

    # Copy main executable changing name to same as in original Nemesis
    copyToMod(config, Path(config.nemesis_program_name), Path(config.nemesis_program_name_in_mod), dst_path_is_file=True)

    # Copy dlls loaded by main executable
    if dlls:
        for file_path in dlls["MSYS"]:
            #shutil.copy2(file_path, config.mod_root_dir)
            copyToMod(config, file_path)
    
    # copy qt windows platform dll
    copyToMod(config, config.msys_prefix / Path("share/qt5/plugins/platforms/qwindows.dll"), Path("platforms"))

    # copy qt imageformats dll
    copyToMod(config, config.msys_prefix / Path("share/qt5/plugins/imageformats/qico.dll"), Path("imageformats"))
    copyToMod(config, config.msys_prefix / Path("share/qt5/plugins/imageformats/qjpeg.dll"), Path("imageformats"))

    # TODO: copy qt imageformats from "share/qt5/plugins/imageformats" in msys

    # make zip version of python library files 
    makePythonLibrary(config)

    
    
        

def main():
    global g_config
    
    arg_parser = argparse.ArgumentParser("Utility for making Nemesis-bfx mod package")
    arg_parser.add_argument("-l", "--includelicenses", action="store_true", help="add licenses from included msys packages")
    args = arg_parser.parse_args()

    if args.includelicenses:
        g_config.include_licenses = True


    if g_config.running_in_msys :
        print("running in MSYS2 : ",g_config.msys, ", root dir = '",g_config.msys_root_dir_win,"'",sep="")
        print("with prefix =", g_config.msys_prefix)
        print("Mod Base directory:", g_config.mod_root_dir)

        makeMod(g_config)



if __name__ == '__main__':
    main()

