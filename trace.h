JET_CALLIN *TraceInit(WCHAR *pszTraceFile);
void TraceExit();
void TraceLog(LPCWSTR pszFormat, ...);
#ifdef _DEBUG
#define DebugTrace(x) TraceLog x
#define BPX __asm {int 3}
#else
#define DebugTrace(x)
#define BPX
#endif
