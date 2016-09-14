copy ..\EnvLayer.np EnvLayer.m
copy ..\NPC0.np NPC0.m
obc EnvLayer.m NPC0.m -o npc0.exe
call BatchSubstitute.bat "(*repl0*)" "(*" NPC0.m > tmp
call BatchSubstitute.bat "(*repl1*)" "*)" tmp > NPC1.np
npc0.exe < NPC1.np
