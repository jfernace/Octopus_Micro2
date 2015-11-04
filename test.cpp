CString s( "abcd" );
#ifdef _DEBUG
afxDump << "CString s " << s << "\n";
#endif
LPTSTR p = s.GetBuffer( 10 );
strcpy( p, "Hello" );   // directly access CString buffer
s.ReleaseBuffer( );
#ifdef _DEBUG
afxDump << "CString s " << s << "\n";
#endif
