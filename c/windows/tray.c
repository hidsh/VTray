#ifdef _WIN32

#include "tray.h"

LRESULT CALLBACK vtray_wndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    struct VTray *tray = (struct VTray *) GetWindowLongPtr(hwnd, GWLP_USERDATA);

    switch (msg) {
        case WM_TRAYICON:
            // Handle tray icon messages here
            if (wParam == ID_TRAYICON) {
                if (lParam == WM_RBUTTONUP) {
                    POINT cursor;
                    GetCursorPos(&cursor);
                    SetForegroundWindow(hwnd);
                    TrackPopupMenu(tray->menu, 0, cursor.x, cursor.y, 0, tray->hwnd, NULL);
                    PostMessage(hwnd, WM_NULL, 0, 0);
                }
            }

            if (wParam == ID_TRAYICON) {
                if (lParam == WM_LBUTTONUP) {
                    exit(1);
                    // MessageBox(hwnd, L"System Tray Icon Clicked!", L"Info", MB_ICONINFORMATION);
                }
            }
            break;

        case WM_COMMAND:
            // Handle menu item selection
            if (HIWORD(wParam) == 0) // Menu item clicked
            {
                int menuId = LOWORD(wParam);
                // Handle menu item action based on menuId
                // You can identify menu items using their IDs.
            }
            break;

        case WM_CLOSE:
            // Handle window close event
            ShowWindow(hwnd, SW_HIDE);
            break;

        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

struct VTray *vtray_init_windows(VTrayParams *params, size_t num_items, struct VTrayMenuItem *items[]) {
    struct VTray *tray = (struct VTray *) malloc(sizeof(struct VTray));
    if (!tray) {
        // Handle allocation failure
        return NULL;
    }

    // Initialize other members of the struct
    memset(tray, 0, sizeof(struct VTray));

    strncpy(tray->identifier, params->identifier, sizeof(tray->identifier));
    // Initialize window class
    memset(&tray->windowClass, 0, sizeof(WNDCLASSEX));
    tray->tooltip = params->tooltip;
    tray->windowClass.cbSize = sizeof(WNDCLASSEX);
    tray->windowClass.lpfnWndProc = vtray_wndProc;
    tray->windowClass.hInstance = tray->hInstance;
    tray->windowClass.lpszClassName = tray->identifier;

    if (!RegisterClassEx(&tray->windowClass)) {
        // Handle class registration failure

        free(tray);
        fprintf(stderr, "Failed to register class\n");

        return NULL;
    }

    // Create a hidden window
    tray->hwnd = CreateWindow(tray->identifier, NULL, 0, 0, 0, 0, 0, NULL, NULL, tray->windowClass.hInstance,
                              NULL);

    if (!tray->hwnd) {
        // Handle window creation failure
        UnregisterClass(tray->identifier, tray->hInstance);
        free(tray);
        fprintf(stderr, "Failed to create window\n");
        return NULL;
    }

    // Initialize the NOTIFYICONDATA structure
    tray->notifyData.cbSize = sizeof(NOTIFYICONDATA);
    wcscpy((wchar_t *) tray->notifyData.szTip, tray->tooltip);
    tray->notifyData.hWnd = tray->hwnd;
    tray->notifyData.uID = ID_TRAYICON;
    tray->notifyData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    tray->notifyData.uCallbackMessage = WM_TRAYICON;
    tray->hInstance = GetModuleHandle(NULL);
    tray->notifyData.hIcon = LoadImageA(tray->hInstance, params->icon, IMAGE_ICON, 0, 0,
                                        LR_LOADFROMFILE | LR_DEFAULTSIZE);
    vtray_construct(items, num_items, tray);
    SetWindowLongPtr(tray->notifyData.hWnd, GWLP_USERDATA, (LONG_PTR) tray);
    if (Shell_NotifyIcon(NIM_ADD, &tray->notifyData) == FALSE) {
        fprintf(stderr, "Failed to register tray icon\n");
        return NULL;
    }
    return tray;
}

void vtray_exit_windows(struct VTray *tray) {
    // Deallocate memory, destroy windows, etc.

    if (tray) {
        if (tray->hwnd)
            DestroyWindow(tray->hwnd);
        if (tray->menu)
            DestroyMenu(tray->menu);
        if (tray->notifyData.hIcon)
            DestroyIcon(tray->notifyData.hIcon);
        UnregisterClass(tray->identifier, tray->hInstance);
        free(tray);
    }
}

void vtray_update_windows(struct VTray *tray) {
    // Update the system tray icon and menu as needed.
    tray->notifyData.hWnd = tray->hwnd;
    Shell_NotifyIcon(NIM_MODIFY, &tray->notifyData);
}

void vtray_construct(struct VTrayMenuItem *items[], size_t num_items, struct VTray *parent) {
    parent->menu = CreatePopupMenu();
    printf("%zu\n", num_items);
    if (parent->menu) {
        for (size_t i = 0; i < num_items; i++) {
            struct VTrayMenuItem *item = items[i];
            MENUITEMINFO menuItem;
            memset(&menuItem, 0, sizeof(MENUITEMINFO));
            menuItem.cbSize = sizeof(MENUITEMINFO);
            menuItem.fMask = MIIM_ID | MIIM_TYPE | MIIM_STATE;
            menuItem.wID = item->id;
            if (item->disabled) {
                menuItem.fState = MFS_DISABLED;
            }

            if (item->toggled) {
                menuItem.fState |= MFS_CHECKED;
            }

            if ((item->image != NULL) && (item->image[0] == '\0')) {
                menuItem.fMask |= MIIM_BITMAP;
                menuItem.hbmpItem = LoadBitmapA(parent->hInstance, item->image);
            }

            if (!AppendMenu(parent->menu, MF_STRING, i, (LPCSTR) item->text)) {
                fprintf(stderr, "Failed to add menu item\n");
                exit(1);
            }
        }
    }
}

void vtray_run_windows(struct VTray *tray) {
    // Show and run your Windows application loop here.
    ShowWindow(tray->hwnd, SW_HIDE);

    // Update the system tray icon
    vtray_update_windows(tray);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

#endif