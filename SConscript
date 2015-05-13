import os
environment = Environment(ENV=os.environ)

lunar = environment.StaticLibrary("lunar", ["LunarGen.c"], CCFLAGS = " -g ")

# A simple exampe of calling into the library directly
lunarflare = environment.Program("lunarflare", ["LunarFlare.c"], LIBS = [lunar])

lunarbootstrap = environment.Program("lunarbootstrap", ["LunarBootstrap.c"], LIBS = [lunar])

libfjcsv = SConscript(dirs = ["libfjcsv"], exports = ["environment"])

bootstrap = environment.Command(["LunarParse.c", "LunarParse.h"], [], "./lunarbootstrap", chdir=os.getcwd())
Depends(bootstrap, lunarbootstrap)
lunarparse = environment.Program("lunarparse", ["LunarParse.c", "LunarCallback.c", "LunarMain.c"], LIBS = [lunar, libfjcsv])

Return("lunar")
