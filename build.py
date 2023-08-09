import os
import sys
import requests

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
    
    # grab stb
    res = requests.get("https://raw.githubusercontent.com/nothings/stb/master/stb_image.h")
    if res.status_code == 200:
        STB_DIR = "./include/stb"
        if not os.path.exists(STB_DIR):
            os.makedirs(STB_DIR)
        with open('./include/stb/stb_image.h', 'wb') as file:
            file.write(res.content)
    else:
        print("failed to grab stb header.. are you connected to the internet?")
        sys.exit()

    os.system('mkdir build')
    os.chdir('./build')
    os.system(f'cmake -DGLEW_DIR={glew_dir} ../')
