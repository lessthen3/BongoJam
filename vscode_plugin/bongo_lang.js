// extension.js
const vscode = require('vscode');
const cp = require('child_process');

function activate(context) {
    const disposable = vscode.commands.registerCommand('bongojam.runFile', () => {

            const editor = vscode.window.activeTextEditor;

            if (!editor) 
            {
                return;
            }

            const file = editor.document.fileName;
            const term = vscode.window.createTerminal('BongoJam');

            term.show();
            term.sendText(`bongo "${file}" --compilerun --debug`);
        }
    );

    context.subscriptions.push(disposable);
}

function deactivate() {}

module.exports = { activate, deactivate };
