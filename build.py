import os
import sys

if __name__ == "__main__":
    if (len(sys.argv) > 2):
        print("too many arguments!")
        sys.exit()
    elif (len(sys.argv) == 1):
        print("too little arguments!")
        sys.exit()

    glew_dir: str = sys.argv[1]

    if (not os.path.exists(glew_dir)):
        print("no such path exists!")
        sys.exit()

    os.system('mkdir build')
    os.chdir('./build')
    os.system(f'cmake -DGLEW_DIR={glew_dir} ../')
