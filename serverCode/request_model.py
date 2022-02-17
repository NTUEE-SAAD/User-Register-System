import requests
import json

BASE_URL = 'http://140.112.174.222:8083'

session = requests.Session()

API_AUTH = '/posts/api/fingerprint'
_data = {'fingerID': 1}
req = session.post(BASE_URL+API_AUTH, json=_data,
                    allow_redirects=True)

jreq = json.loads(req.text)
print("Get request", jreq)