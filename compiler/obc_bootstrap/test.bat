npc0.exe < ..\Test%1%.np
del Test%1%.npx
ren out Test%1%.npx
..\..\runtime\npx.exe Test%1%.npx
