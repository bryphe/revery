#include "config.h"
#ifdef USE_WIN32

#include <stdio.h>

//#include "win32_target.h"
#include <Shlobj.h>
#include <Shobjidl.h>
#include <Windows.h>
#include <winuser.h>

void revery_alert_win32(void *pWin, const char *szMessage) {
  HWND hwnd = (HWND)pWin;

  MessageBox(hwnd, szMessage, "Alert", MB_ICONWARNING | MB_OK);
}

const char **revery_open_files_win32(const char *startDir, int canChooseFiles,
                                     int canChooseDirectories,
                                     const char *title) {
  (void)startDir;
  (void)canChooseFiles;

  // Default return - null
  const char **ret = malloc(1 * sizeof(char *));
  ret[0] = NULL;

  if (canChooseDirectories) {
    HRESULT hr = CoInitialize(NULL);
    if (SUCCEEDED(hr)) {
      TCHAR szDir[MAX_PATH];
      BROWSEINFO bInfo;
      bInfo.hwndOwner = NULL;
      bInfo.pidlRoot = NULL;
      bInfo.pszDisplayName =
          szDir; // Address of a buffer to receive the display name of the
                 // folder selected by the user
      bInfo.lpszTitle = title;
      bInfo.ulFlags = BIF_USENEWUI | BIF_NEWDIALOGSTYLE;
      bInfo.lpfn = NULL;
      bInfo.lParam = 0;
      bInfo.iImage = -1;

      LPITEMIDLIST lpItem = SHBrowseForFolder(&bInfo);
      if (lpItem != NULL) {

        if (SHGetPathFromIDList(lpItem, szDir)) {
          printf("OUT: %s\n", szDir);
          free(ret);
          ret = malloc(2 * sizeof(char *));
          ret[0] = strdup(szDir);
          ret[1] = NULL;
        }
      }
    }

  } else {

    char *filter = "All Files (*.*)\0*.*\0";
    OPENFILENAME ofn;
    char fileName[MAX_PATH] = "";
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = NULL;
    ofn.lpstrFilter = filter;
    ofn.lpstrFile = fileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
    ofn.lpstrDefExt = "";
    if (GetOpenFileName(&ofn)) {
      free(ret);
      ret = malloc(2 * sizeof(char *));
      ret[0] = strdup(fileName);
      ret[1] = NULL;
    }
  }
  return ret;
}
#endif
