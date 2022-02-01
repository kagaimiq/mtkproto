import sys

if len(sys.argv) <= 4:
	print("Usage: %s <loader file> <kernel file> <dtb file> <output paylaod>" % sys.argv[0])
	sys.exit(1)

with open(sys.argv[4], 'wb') as outf:
	with open(sys.argv[1], 'rb') as inpf:
		payload_data = bytearray(inpf.read())
	
	with open(sys.argv[2], 'rb') as inpf:
		kernel_data = inpf.read()
	
	with open(sys.argv[3], 'rb') as inpf:
		dtb_data = inpf.read()
	
	#kernel_data = b'\x0e\xf0\xa0\xe1EBINA EBINA'
	#dtb_data = b'\xd0\x0d\xfe\xed\x00\x00\0x00\0x48-------WAKUWAKU-[RTL-SDR//RTL-SDR]-K262m1_H11r291-|-|-|-|-|-|-|-'
	
	#####################################
	
	plinfo_pos = payload_data.find(b'Ebina!')
	payload_data[plinfo_pos+6:plinfo_pos+10] = int.to_bytes(len(dtb_data), 4, 'big')
	payload_data[plinfo_pos+10:plinfo_pos+14] = int.to_bytes(len(kernel_data), 4, 'big')
	
	outf.write(payload_data)
	
	outf.seek((outf.tell() + 0xff) & ~0xff)
	print('%x' % outf.tell())
	outf.write(kernel_data)
	
	outf.seek((outf.tell() + 0xff) & ~0xff)
	print('%x' % outf.tell())
	outf.write(dtb_data)
