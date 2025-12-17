savedcmd_memstat.mod := printf '%s\n'   memstat.o | awk '!x[$$0]++ { print("./"$$0) }' > memstat.mod
