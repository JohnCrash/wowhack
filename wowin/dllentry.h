#pragma once

void RelpaceEntry(PCSTR pszCalleeModName,
				  PROC pfnCurrent,PROC pfnNew,
				  HMODULE hmodCaller);