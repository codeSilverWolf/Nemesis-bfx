

from pathlib import Path, PurePath, PurePosixPath, WindowsPath
import os
import shutil
import subprocess
import re

# global config, instance of class MakeModConfig
g_config = None

class MakeModConfig:
    def __init__(self) -> None:
        self.nemesis_program_name = "NemesisUnlimitedBehaviorEngine.exe"        # compiled executable name
        self.work_dir = Path.cwd()      # current working directory, should point to path to build dir containing compiled executable
        self.mod_root_dir = self.work_dir / "Archnemesis/Data/Nemesis_Engine"
        self.msys_root_dir_win = Path("c:/msys64") # default , should be overwritten
        if "MSYSTEM" in os.environ :
            self.running_in_msys = True     # are we running inside msys2 shell?
            self.msys = os.environ["MSYSTEM"]
            self.msys_prefix = os.environ["MSYSTEM_PREFIX"]
            self.msys_root_dir_win = getMsysRootDir()   # get root path for current msys environment (windows compatible path)
        else:
            self.running_in_msys = False

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

def msysPathToWindows(path):
    """
    translate msys unix path to corresponding windows path
    """
    global g_config

    if not isinstance(path, Path):
        path = Path(path)
    wp = g_config.msys_root_dir_win / (path.relative_to("/"))
    return wp
    

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
                    print("error: unable to match needed dll to category. dll=[",dll_full_path,']',sep='')

        return dlls
    else:
        return None

def copyToMod(config: MakeModConfig, src_path: Path, dst_path: Path = Path("."), create_dirs = True) -> bool:
    """
    copy file or directory to directory relative to mod root path, overwriting destination files when necessary

    """
    # convert unix path to windows
    if type(src_path) is PurePosixPath:
        if src_path.root == '/':
            src_path = config.msys_root_dir_win / src_path.relative_to('/')
    
    if not src_path.exists():
        print("ERROR: cannot copy - file", str(src_path), "does not exists!")
        return False
    if dst_path.root or dst_path.drive:
        print("ERROR: cannot copy - file ", str(src_path), " Destination path '", dst_path,"' has to be relative!")
    
    dst_path = config.mod_root_dir / dst_path
    # dst_is_directory = False
    # if dst_path.name == "*":
    #     # treat dst path as directory
    #     dst_path = dst_path.parent
    #     dst_is_directory = True
    
    if not dst_path.exists():
        if create_dirs:
            dst_path.mkdir(parents=True)
            print("making path:", str(dst_path))
        else:
            print("ERROR: destinition directory ", str(dst_path), "does not exists!")
            return False
    if dst_path.is_dir():
        file_was_overwritten = False # was the file overw
        skip_file = False       # if dst file is same/newer - skip it
        # check if destination file exists and delete if older/different
        dst_file = dst_path / src_path.name
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
            rv = shutil.copy2(src_path, dst_path)
            print("OVERWRITE" if file_was_overwritten else "COPY", str(src_path), "->", str(rv))

    return True
    



def makeMod(config: MakeModConfig):

    nem_path = Path(config.nemesis_program_name)
    if not nem_path.is_file:
        print("ERROR: Nemesis executable '",nem_path,"'not found. Current work directory:", Path.cwd())
        return None
    print("Found menesis executable. Building mod directory.")
    dlls = getDllsFromExecutable(config, config.nemesis_program_name)

    if not config.mod_root_dir.exists():
        config.mod_root_dir.mkdir(parents=True)

    # copy files
    # shutil.copy2(config.nemesis_program_name, config.mod_root_dir)
    copyToMod(config, Path(config.nemesis_program_name))

    if dlls:
        for file_path in dlls["MSYS"]:
            #shutil.copy2(file_path, config.mod_root_dir)
            copyToMod(config, file_path)


    
        

def main():
    global g_config
    g_config = MakeModConfig()


    if g_config.running_in_msys :
        print("running in MSYS2 : ",g_config.msys, ", root dir = '",g_config.msys_root_dir_win,"'",sep="")
        print("with prefix =", g_config.msys_prefix)
        print("Mod Base directory:", g_config.mod_root_dir)

        #print("dlls:", getDllsFromExecutable(g_config, g_config.nemesis_program_name))

        makeMod(g_config)



if __name__ == '__main__':
    main()

