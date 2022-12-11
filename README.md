# WSL2 network Settings
- **Port Forwarding**: exec $PATH		(wsl-connect-external.ps1 file)
- **Check**: netsh interface portproxy show all
- **Delete**: netsh interface portproxy delete v4tov4 listenaddress=$IP listenport=$PORT
