
#include <tchar.h>
#include <windows.h>
#include <winuser.h>

const int MaxExtendedVKeys = 10;
const BYTE ExtendedVKeys[MaxExtendedVKeys] = {VK_UP, VK_DOWN, VK_LEFT, VK_RIGHT, VK_HOME, VK_END, VK_PRIOR, /* PgUp */ VK_NEXT, /* PgDn */ VK_INSERT, VK_DELETE};

// Checks whether the specified VKey is an extended key or not
bool IsVkExtended(BYTE VKey)
{
  for (int i = 0; i < MaxExtendedVKeys; i++)
  {
    if (ExtendedVKeys[i] == VKey)
      return true;
  }
  return false;
}

void KeyboardEvent(BYTE VKey, BYTE ScanCode, LONG Flags)
{
  // MSG KeyboardMsg;
  keybd_event(VKey, ScanCode, Flags, 0);
  /*
    while (PeekMessage(&KeyboardMsg, 0, WM_KEYFIRST, WM_KEYLAST, PM_REMOVE))
    {
      TranslateMessage(&KeyboardMsg);
      DispatchMessage(&KeyboardMsg);
    }*/
}

void SendKeyUp(BYTE VKey)
{
  BYTE ScanCode = LOBYTE(MapVirtualKey(VKey, 0));
  KeyboardEvent(VKey, ScanCode, KEYEVENTF_KEYUP | (IsVkExtended(VKey) ? KEYEVENTF_EXTENDEDKEY : 0));
}

void SendKeyDown(BYTE VKey)
{
  BYTE ScanCode = 0;
  ScanCode = LOBYTE(MapVirtualKey(VKey, 0));
  KeyboardEvent(VKey, ScanCode, IsVkExtended(VKey) ? KEYEVENTF_EXTENDEDKEY : 0);
  KeyboardEvent(VKey, ScanCode, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP);
}
