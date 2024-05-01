Import("env")

env.Prepend(UPLOADERFLAGS=[env.subst("$UPLOADER")])
env.Replace(UPLOADER="esptool-ftdi.py")
