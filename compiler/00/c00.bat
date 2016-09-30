copy EnvLayer.np obc_bootstrap\EnvLayer.m
copy NPC00.np obc_bootstrap\NPC0.m
cd obc_bootstrap
obc EnvLayer.m NPC0.m -o npc00.exe
move npc00.exe ..
cd ..
