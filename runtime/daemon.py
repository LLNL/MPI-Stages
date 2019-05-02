import os
import select
import socket
import logging

errorcodes = {
	'MPI_SUCCESS': 0,
	'MPI_ABORT': 255,
	'MPI_FAULT': 124,
	'MPIX_TRY_RELOAD': 101,
	'SEGFAULT': -11
}
errornames = {v: k for k, v in errorcodes.iteritems()}

class Daemon:
	def __init__(self, port):
		self.setup_logger()
		self.log.debug('logger set up')

		self.setup_signal_handlers()
		self.log.debug('signal handlers set up')

		self.sockets = []
		self.packet_switcher = {}
		self.packet_size = 64

		self.port = port
		self.setup_daemon_socket()

	def __del__(self):
		self.log.debug('Daemon.__del__ entry')

		self.log.debug('closing all sockets')
		for socket in self.sockets:
			socket.close()

		self.log.debug('Daemon.__del__ exit')

	def setup_logger(self):
		raise NotImplementedError

	def setup_signal_handlers(self):
		raise NotImplementedError

	def setup_daemon_socket(self):
		# cache network data
		self.hostname = socket.gethostname()
		self.host = socket.gethostbyname(self.hostname)

		self.daemon_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		self.daemon_socket.setblocking(0)
		
		self.daemon_socket.bind((self.host, self.port))
		self.daemon_socket.listen(5)

		# give for regular checking
		self.sockets.append(self.daemon_socket)

	def add_handle(self, protocol, handle):
		self.packet_switcher[protocol] = handle

	def launch_child_process(self):
		raise NotImplementedError

	def launch(self):
		self.log.debug('Daemon.launch entry')
		
		# TODO launch controller
		#self.launch_child_process()

		self.shutdown = False
		while not self.shutdown:
			# TODO should we make use of writables and exceptionals?
			readables, writables, exceptionals = select.select(self.sockets, [], [])

			for readable in readables:
				#if readable is self.child interface for connection
				#else:
					# msg

				packet = readable.recv(self.packet_size)
				
				# avoid empty packets
				if packet:
					packet = packet.decode('utf-8').replace('\0','')

					self.log.debug('packet received from ' + str(readable.getpeername()))
					self.log.debug('packet: "' + str(packet) + '"')
					
					contents = packet.split(' ')

					# find correct handler for packet
					packet_handler = self.packet_switcher[contents[0]]
					if packet_handler:
						self.log.debug('packet handler: ' + str(packet_handler))
						
						# let packet_handler deal with packet
						packet_handler(readable, contents[1:])

					else:
						self.log.error('failed to find handler for packet')
						raise ValueError

		self.log.debug('Daemon.launch exit')
					
	# send to controller/head/rank/fault
