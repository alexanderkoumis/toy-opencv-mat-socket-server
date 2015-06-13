import os
import sys
import glob

def main():

    supported_exts = ['jpg', 'png']
    images_found = False

    num_args = len(sys.argv)

    if num_args < 2:
        print 'Error in ' + __file__ + ': Wrong number of arguments'
        print 'Number of arguments ' + str(num_args)
        arg_cnt = 0
        for arg in sys.argv:
            print 'arg #' + str(arg_cnt) + ': ' + arg
        exit(1)

    pic_dir = sys.argv[1]

    for ext in supported_exts:
        files_sorted = sorted(glob.glob(pic_dir + '/*.' + ext))
        if len(files_sorted) > 0:
            for file_sorted in files_sorted:
                file_base = os.path.basename(file_sorted)
                print file_base

if __name__ == "__main__":
    main()