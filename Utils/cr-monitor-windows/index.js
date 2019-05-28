const { app, BrowserWindow } = require('electron');

let win = null;

const appUrl = `http://monitor.credits.com/`;

function createElectronShell() {
    win = new BrowserWindow(
        {
            width: 1000,
            height: 900,
            webPreferences: {
                plugins: true,
                nodeIntegration: false,
                webSecurity: false,
                allowDisplayingInsecureContent: true,
                allowRunningInsecureContent: true
            }
        }
    );

    win.setMenu(null);
    win.loadURL(appUrl);
    win.on('closed', () => {
        win = null
    });
}

app.commandLine.appendSwitch('ignore-certificate-errors', 'true');

app.on('ready', createElectronShell);

app.on('browser-window-created',function(e,window) {
    window.setMenu(null);
});

app.on('window-all-closed', () => {
    if (process.platform !== 'darwin') app.quit();
});

app.on('activate', () => {
    if (win == null) createElectronShell();
});