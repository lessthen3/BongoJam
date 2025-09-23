const cp = require ('child_process');

const 
{
    createConnection,
    TextDocuments,
    ProposedFeatures,
} = require('vscode-languageserver');


const 
{ 
    LanguageClient, 
    LanguageClientOptions, 
    ServerOptions, 
    TransportKind, 
    RevealOutputChannelOn, 
    ErrorAction, 
    CloseAction 
} = require('vscode-languageclient');

//import * as path from 'path'; // If you need to resolve paths

function checkPythonInstallation() 
{
    return new Promise
    ((resolve) => 
        {
            cp.exec
            (
                'python --version', (error, stdout) => 
                {
                    if (error) 
                    {
                        vscode.window.showErrorMessage('Python is required for this extension, but it was not found in your PATH!');
                        resolve(false);
                    } 
                    else 
                    {
                        resolve(true);
                    }
                }
            );
        }
    );
}

async function activate(context) 
{

    const IsPythonAvailable = await checkPythonInstallation();

    if (!IsPythonAvailable) 
    {
        return; // Stop activation if Python is not available
    }

    //const serverModulePath = context.asAbsolutePath(path.join('server', 'LanguageServer.py'));

    const ServerOptions = 
    {
        run: { command: 'python', args: ['LanguageServer.py'] },
        debug: { command: 'python', args: ['LanguageServer.py', '--debug'] }
    };

    const ClientOptions = 
    {
        documentSelector: [{ scheme: 'file', language: 'bongojam' }],

        revealOutputChannelOn: RevealOutputChannelOn.Error, // Show output channel on errors
        
        errorHandler: 
        {
            error: (error, message, count) => 
            {
                vscode.window.showErrorMessage(`Language server error: ${message.reason}`);
                return ErrorAction.Continue;
            },
            closed: () => 
            {
                vscode.window.showInformationMessage('The language server crashed and will restart.');
                return CloseAction.Restart;
            }
        },

        // synchronize: 
        // {
        //     fileEvents: vscode.workspace.createFileSystemWatcher('**/*.bj')
        // }
    };

    const Client = new LanguageClient
    (
        'BongoJam_Language_Server',
        'BongoJam Language Server',
        ServerOptions,
        ClientOptions
    );

    Client.start();
    context.subscriptions.push(Client);
}

function deactivate() 
{
    if (client) 
    {
        return client.stop();
    }
}

module.exports = 
{
    activate,
    deactivate
};