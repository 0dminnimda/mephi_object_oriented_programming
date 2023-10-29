import subprocess
import sys
import os

# env = os.environ.copy()

# banned = {
#     "MINGW_PACKAGE_PREFIX",
#     "PKG_CONFIG_PATH",
#     "PKG_CONFIG_SYSTEM_LIBRARY_PATH",
#     "ACLOCAL_PATH",
#     "ORIGINAL_PATH",
#     "TERM",
#     "MSYS_NO_PATHCONV",
#     "MINGW_PREFIX",
#     "SHELL",
#     "MINGW_CHOST",
#     "CONFIG_SITE",
#     "DISPLAY",
#     "PKG_CONFIG_SYSTEM_INCLUDE_PATH",
#     "MSYSTEM_PREFIX",
#     "MSYSTEM_CARCH",
#     "MSYSTEM",
#     "MSYSTEM_CHOST",
#     "ORIGINAL_TEMP",
#     "OLDPWD",
#     "ORIGINAL_TMP",
#     "PWD",
#     "EXEPATH",
#     "INFOPATH",
#     "SSH_ASKPASS",
#     "MANPATH",
#     "TMPDIR",
#     "HOSTNAME",
#     "HOME",
#     "_",
#     "PLINK_PROTOCOL",
#     "SHLVL",
# }

# for name in banned:
#     env.pop(name, None)

# env["SHELL"] = "cmd.exe"


# def hook(event, args):
#     if event == 'subprocess.Popen':
#         print(f'audit: {event} with args={args}')


# sys.addaudithook(hook)
# subprocess.Popen([sys.executable, "-m", *sys.argv[1:]], shell=False)

# subprocess.Popen(["cmd.exe", "/C", *(sys.argv[1:])], env=env)
subprocess.Popen([*sys.argv[1:]], shell=True)

# import os, psutil; pprocName = psutil.Process(os.getppid()).name()
