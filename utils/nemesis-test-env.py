from pathlib import Path, PurePath, PurePosixPath, WindowsPath
import os
import shutil
import subprocess
import re
import argparse

def copyDirTree(src_path: Path, dst_path: Path, copy_masks = [('INCLUDE', r'*')], create_dirs = True, recurse = True, ask_confirmation = False) -> bool:
    """
    copy directory tree starting from src_path to directory pointed by dst_path
    files to copy are selected by entries in copy_masks list.
    copy_mask = [ tuple ( 'INCLUDE' | 'EXCLUDE' | 'EXCLUDE_DIR, 'glob-like expression matching filenames')]
    list is processed in order, if no 'INCLUDE' match is found file does NOT get copied
    EXCLUDE_DIR skips matching directories
    """
    # convert paths to pathlib.Path if they're strings
    if type(src_path) == str:
        src_path = Path(src_path)
    if type(dst_path) == str:
        dst_path = Path(dst_path)

    # ask for confirmation
    if ask_confirmation:
        print("do you want to copy files from:", src_path, "to:", dst_path, " ?")
        print("with mask:", copy_masks)
        while response := input("(Y/n)?> ").lower():
            if response == "n":
                return False
            elif response == "y":
                break
            else:
                print("Invalid input! Expected 'Y(y)' or 'N(n)' only")
        

    if not src_path.exists():
        print("ERROR: cannot copy - file", str(src_path), "does not exists!")
        return False
    print("copyDir entering:", str(src_path))

    # iterate over directory content
    with os.scandir(src_path) as dir_it:
        for entry in dir_it:
            if entry.is_file():
                # match against copy_masks
                for copy_mask_type, copy_mask in copy_masks:
                    if Path(entry.name).match(copy_mask):
                        if copy_mask_type == 'INCLUDE':
                            # create destination path
                            if not dst_path.exists():
                                if create_dirs:
                                    dst_path.mkdir(parents=True)
                                    print("making path:", str(dst_path))
                                else:
                                    print("ERROR: destinition directory ", str(dst_path), "does not exists!")
                                    return False

                            src_file_path = src_path / entry.name
                            print("COPY", str(src_file_path), "->", str(dst_path))
                            rv = shutil.copy2(src_file_path, dst_path)
                        elif copy_mask_type == 'EXCLUDE':
                            # file is excluded, skip next rules
                            print("SKIP", str(src_path / entry.name))
                            break
            elif entry.is_dir() and recurse:
                # recurse into directory if not in excluded directories
                ok_to_descend = True
                for copy_mask_type, copy_mask in copy_masks:
                    if copy_mask_type == 'EXCLUDE_DIR' and Path(entry.name).match(copy_mask):
                        ok_to_descend = False
                        break

                if ok_to_descend:
                    copyDirTree(src_path/entry.name, dst_path/entry.name, copy_masks)

    return True






if __name__ == "__main__":

    arg_parser = argparse.ArgumentParser("Utility for making Nemesis test environments, and testing results")
    arg_parser.add_argument("src", help="full path to skyrim directory - where SkyrimSE.exe exists")
    arg_parser.add_argument("dst", help="full path to directory where test directory hierarchy will be created")
    arg_parser.add_argument("-i", "--interactive", action="store_true", help="ask for confirmations before copying file groups")
    args = arg_parser.parse_args()

    print("Skyrim directory:", args.src)
    print("Test environment directory", args.dst)

    ask_confirmation = False
    if args.interactive:
        print("Interactive mode enabled.")
        ask_confirmation = True

    if args.src and args.dst:
        # TODO: check if src dir is skyrim directory

        # test_dir_path - destinition directory for test environment. It has to have  "project new reign - nemesis/test environment" at end of path
        # nemesis executables and files go straight to this directory.
        # In "real" skyrim directory Nemesis goes to "Data/Nemesis_Engine" directory.
        # Required files from skyrim "Data" dir go to "data" dir inside test directory.
        test_dir_path = Path(args.dst) / Path("project new reign - nemesis/test environment")

        # copy all  animation data single files from "Data/Meshes/" directory
        copy_masks = [("INCLUDE", "*animation*file.txt")]
        dir_to_copy = Path("Data/Meshes")

        copyDirTree(Path(args.src)/dir_to_copy, test_dir_path/dir_to_copy, copy_masks=copy_masks, recurse=False, ask_confirmation=ask_confirmation) # do not recurse!

        # copy all  *.txt, *.xml and *.hkx files from "Data/Meshes/actors" directory
        copy_masks = [("INCLUDE", "*.txt"), ("INCLUDE", "*.xml"), ("INCLUDE", "*.hkx")]
        dir_to_copy = Path("Data/Meshes/actors")

        copyDirTree(Path(args.src)/dir_to_copy, test_dir_path/dir_to_copy, copy_masks=copy_masks, ask_confirmation=ask_confirmation)

        # TODO: check if nemesis engine exists in skyrim game directory
        # copy all files from "Data/Nemesis_Engine" directory to test environment ROOT path!!! not to Data
        # except nemesis main exe files and libraries, and included python
        copy_masks = [("EXCLUDE_DIR", "Lib"), # exclude python lib
                        ("EXCLUDE", "python3?.dll"),
                        ("EXCLUDE", "nemesis.ini"), # important - game directory is in stored this file TODO: parse file and chande directory to test env.
                        ("EXCLUDE_DIR", "platforms"), ("EXCLUDE_DIR", "imageformats"), ("EXCLUDE", "Qt5*.dll"), # exclude Qt libs
                        ("EXCLUDE", "Nemesis Unlimited Behavior Engine.exe"), # exclude main executable
                        ("INCLUDE", "*")]
        dir_to_copy = Path("Data/Nemesis_Engine")

        copyDirTree(Path(args.src)/dir_to_copy, test_dir_path, copy_masks=copy_masks, ask_confirmation=ask_confirmation)

        print("Done creating test environment.")
