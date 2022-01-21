from bluepy.btle import Peripheral, UUID
from bluepy.btle import Scanner

scanner = Scanner()
print("Scanning for 5 seconds...")
devices = list(scanner.scan(5.0))

for i,dev in enumerate(devices):
    print("#%d: %s (%s), RSSI=%d dB" % (i, dev.addr, dev.addrType, dev.rssi))
    for (adtype, desc, value) in dev.getScanData():
        if desc=="Complete Local Name":
            print( "  %s = %s" % (desc, value))

print()
number = int(input('Enter your device number: '))
print(f'Connecting to #{number}: {devices[number].addr}')

dev = Peripheral(devices[number].addr, devices[number].addrType)

print()
print("Available Services:")
for svc in dev.services:
    print("Service", str(svc))
    
try:
    testService= dev.getServiceByUUID(UUID(0xfff0))
    for ch in testService.getCharacteristics():
        print(str(ch))
    ch= dev.getCharacteristics(uuid=UUID(0xfff4))[0]
    
    print()
    if (ch.supportsRead()):
        print("Data in channel:", ch.read())
    if input("Write?"):
        write = input("To Write:")
        ch.write(write.encode('utf-8'))

finally:
    dev.disconnect()
