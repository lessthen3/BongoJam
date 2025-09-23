import argparse
from pygls.server import LanguageServer
from pygls.features import COMPLETION, HOVER
from pygls.types import CompletionParams, HoverParams, CompletionList, CompletionItem, Hover, MarkupContent

class BongoJamLanguageServer(LanguageServer):
    def __init__(self):
        super().__init__()

def main():
    parser = argparse.ArgumentParser(description='Start BongoJam Language Server')
    parser.add_argument('--stdio', action='store_true', help='Use stdio for communication')
    parser.add_argument('--debug', action='store_true', help='Use debug for debugging')
    
    args = parser.parse_args()

    server = BongoJamLanguageServer()

    @server.feature(COMPLETION, trigger_characters=['.', ':'])
    def completions(params: CompletionParams) -> CompletionList:
        """Returns completion items."""
        return CompletionList(is_incomplete=False, items=[
            CompletionItem(label='Type here'),
        ])

    @server.feature(HOVER)
    def hover(params: HoverParams) -> Hover:
        """Returns hover information."""
        return Hover(contents=MarkupContent(kind='markdown', value='Hover content here'))

    if args.stdio:
        server.start_io()
    else:
        # Optional: Add different startup modes or log a message stating that stdio is required.
        print("Error: --stdio flag is required to start the server.")
        # Here you might also handle other communication protocols or exit if stdio is mandatory.

if __name__ == '__main__':
    main()
