import sys, struct

if len(sys.argv) < 5:
	print("Usage: %s <source file> <name> <load address> <output file>")
	sys.exit(1)

with open(sys.argv[4], 'wb') as outf:
	with open(sys.argv[1], 'rb') as inpf:
		source_data = inpf.read()
	
	# header
	outf.write(struct.pack('<II32sI', 0x58881688, len(source_data), bytes(sys.argv[2], 'ascii'), int(sys.argv[3], 0)))
	
	# padding (what's better technique to pad with some value ???)
	outf.write(b'\xff' * ((0x200 - (outf.tell() & 0x1ff)) & 0x1ff))
	
	# data
	outf.write(source_data)