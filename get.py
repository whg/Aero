
import httplib2
import os
import json
from collections import defaultdict

from apiclient import discovery
from oauth2client import client
from oauth2client import tools
from oauth2client.file import Storage

SCOPES = 'https://www.googleapis.com/auth/spreadsheets.readonly'
CLIENT_SECRET_FILE = 'client_secret.json'
APPLICATION_NAME = 'Google Sheets API Python for stillatplay Work'
CREDENTIAL_PATH = 'sheets.googleapis.com-python-stillatplay-work.json'

def get_credentials():
    """Gets valid user credentials from storage.

    If nothing has been stored, or if the stored credentials are invalid,
    the OAuth2 flow is completed to obtain the new credentials.

    Returns:
        Credentials, the obtained credential.
    """

    store = Storage(CREDENTIAL_PATH)
    credentials = store.get()
    if not credentials or credentials.invalid:
        flow = client.flow_from_clientsecrets(CLIENT_SECRET_FILE, SCOPES)
        flow.user_agent = APPLICATION_NAME
        credentials = tools.run_flow(flow, store)
        print('Storing credentials to ' + CREDENTIAL_PATH)
    return credentials

def get():
    """Grab the data!
    """
    credentials = get_credentials()
    http = credentials.authorize(httplib2.Http())
    discoveryUrl = ('https://sheets.googleapis.com/$discovery/rest?'
                    'version=v4')
    service = discovery.build('sheets', 'v4', http=http,
                              discoveryServiceUrl=discoveryUrl)

    spreadsheetId = '1ii-KUiV7Gi7C0jeePzzl2I-EBDMF8CKnLT8bE-iVhqw'
    rangeName = 'Sheet1!B2:AC35'
    result = service.spreadsheets().values().get(
        spreadsheetId=spreadsheetId, range=rangeName).execute()
    values = result.get('values', [])

    return values

def arrange(raw, width=28, height=17):
    cells = defaultdict(dict)
    
    for y, row in enumerate(raw):
        for x, cell in enumerate(row):
            if cell:
                index = (x, y // 2)
                if y % 2 == 0:
                    cells[index]['x'] = x
                    cells[index]['y'] = y // 2
                    cells[index]['name'] = cell
                else:
                    try:
                        box, bank, chan = [int(p) - 1 for p in cell.split('.')]
                        addr =  box * 48 + bank * 3 + chan
                    except ValueError:
                        addr = 300
                    cells[index]['addr'] = addr
                    print(cell, box, bank, chan, addr)

    return cells


if __name__ == '__main__':
    raw = get()
    matrix = arrange(raw)
    filepath = 'assets/mapping.json'
    with open(filepath, 'w') as f:
        json.dump([dict(v) for v in matrix.values()], f, indent=True)
    print('wrote output to {}'.format(filepath))
    
    
