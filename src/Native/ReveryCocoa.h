void revery_alert_cocoa(void *pWin, const char *szMessage);
char **revery_open_files_cocoa(const char *startDir, const char *fileTypes[],
                               int fileTypesSize, int allowMultiple,
                               int canChooseFiles, int canChooseDirectories,
                               int showHidden, const char* buttonText,
                               const char* title);

/* Notification functions */
void revery_dispatchNotification_cocoa(const char *title, const char *body,
                                       long onClickFunc, int mute);
void revery_scheduleNotificationFromNow_cocoa(const char *title,
        const char *body,
        long onClickFunc, int mute,
        int seconds);
void *revery_getIconHandle_cocoa();

/* Icon progress bar functions */
void revery_setIconProgress_cocoa(void* dt, double progress);
void revery_setIconProgressIndeterminate_cocoa(void *dt);
void revery_hideIconProgress_cocoa(void* ip);

/* Tray */
void *revery_setTrayTitle_cocoa(void *nsStatusItem, const char *titleText);
void revery_removeStatusItem_cocoa(void* nsStatusItem);

/* Image functions */
void *revery_makeImageFromAbsolutePath_cocoa(const char *image_path_v);

/* Open functions */
int revery_openURL_cocoa(const char *url_string);
int revery_openFile_cocoa(const char *path_string);

/* Locale functions */
char *revery_getUserLocale_cocoa();

/* Menu functions */
void *revery_getMenuBarHandle_cocoa();
void *revery_menuCreate_cocoa(const char *title);
void *revery_menuItemCreate_cocoa(const char *title, void *keyEquivalent);
void *revery_menuNth_cocoa(void *nsMenu, int idx);
void revery_menuAddItem_cocoa(void *nsMenu, void *nsMenuItem);
void *revery_menuItemGetSubmenu_cocoa(void *nsMenuItem);
void revery_menuAddSubmenu_cocoa(void *parent, void *child);
void revery_menuRemoveSubmenu_cocoa(void *parent, void *child);
void revery_menuRemoveItem_cocoa(void *nsMenu, void *nsMenuItem);
void revery_menuInsertItemAt_cocoa(void *nsMenu, void *nsMenuItem, int idx);
void revery_menuInsertSubmenuAt_cocoa(void *parent, void *child, int idx);
void revery_menuClear_cocoa(void *nsMenu);
void *revery_menuItemCreateSeparator_cocoa();
void revery_menuItemSetEnabled_cocoa(void *menuItem, int truth);
void revery_menuItemSetVisible_cocoa(void *menuItem, int truth);
void revery_menuDisplayIn_cocoa(void *nsMenu, void *nsWindow, int x, int y);

/* Input functions */
void *revery_buttonCreate_cocoa(const char *title);
void revery_buttonSetColor_cocoa(void *nsButton, double red, double green, double blue, double alpha);
void revery_openEmojiPanel_cocoa(void);

/* Window functions */
void revery_windowSetUnsavedWork_cocoa(void *memory, int truth);
