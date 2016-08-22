copy ..\EnvLayer.np EnvLayer.m
copy ..\NPC0.np NPC0.m
obc EnvLayer.m NPC0.m -o npc0.exe
npc0.exe < ..\NPC0.np
