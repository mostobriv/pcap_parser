# pcap_parser
Pcap parser mostly made for pahan tool

## Requirements
`apt install libpcap-dev libboost-stacktrace-dev libpqxx-dev`

# TODO:
- [X] ~~Make skeleton~~
- [ ] Complete first version of parser module
  - LRU cache with pg pipelining?
  - DB uploading in connection_end callback
- [ ] Logger module (it looks terrible atm)
- [ ] Watcher of fs (inotify)
- [ ] Database communication
