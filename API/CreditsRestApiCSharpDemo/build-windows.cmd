git clone https://github.com/CREDITSCOM/thrift-interface-definitions
thrift -gen csharp -out . .\thrift-interface-definitions\general.thrift
thrift -gen csharp -out . .\thrift-interface-definitions\api.thrift
dotnet add package apache-thrift-netcore --version 0.9.3.2
dotnet add package SimpleBase --version 1.8.0
pause