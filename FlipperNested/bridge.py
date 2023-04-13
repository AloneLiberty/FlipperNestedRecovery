import serial
import serial.tools.list_ports
from google.protobuf.internal.encoder import _VarintBytes
from google.protobuf.json_format import MessageToDict

import FlipperNested.proto.flipper_pb2 as flipper_pb2
import FlipperNested.proto.storage_pb2 as storage_pb2


class FlipperBridge:
    def __init__(self):
        self.device_info = None
        self._serial = None
        self.command_id = 0
        self.connect()

    def connect(self):
        self._serial = self.open_port()
        self.start_rpc_session()

    def open_port(self):
        port = self.get_port()
        if not port:
            raise ConnectionError("Flipper is missing")
        flipper = serial.Serial(port, timeout=1)
        flipper.flushOutput()
        flipper.flushInput()

        return flipper

    def start_rpc_session(self) -> None:
        self._serial.read_until(b">: ")
        self._serial.write(b"start_rpc_session\r")
        self._serial.read_until(b"\n")

    def get_port(self):
        ports = serial.tools.list_ports.comports()
        potential_ports = []
        for port, desc, hwid in ports:
            a = hwid.split()
            if "VID:PID=0483:5740" in a:
                return port
            elif hwid == "n/a":
                potential_ports.append(port)
        for port in potential_ports:
            if self.check_port(port):
                return port
        if len(potential_ports):
            print("[Error] Can't guess Flipper Zero serial port. Fallback to manual mode")
            print("Make sure Flipper Zero is connected and not used by any other software")
            for port in potential_ports:
                if input("Is {} your Flipper Zero serial port? [y/n] > ".format(port)).lower() == "y":
                    return port

    @staticmethod
    def check_port(port):
        try:
            flipper = serial.Serial(port, timeout=1)
            flipper.flushOutput()
            flipper.flushInput()
            if b"Flipper Zero Command Line Interface!" in flipper.read_until(b">: "):
                return True
        except:
            pass

    def _read_varint_32(self) -> int:
        MASK = (1 << 32) - 1

        result = 0
        shift = 0
        while 1:
            b = int.from_bytes(self._serial.read(size=1), byteorder="little", signed=False)
            result |= (b & 0x7F) << shift

            if not b & 0x80:
                result &= MASK
                result = int(result)
                return result
            shift += 7

    def get_command_id(self) -> int:
        self.command_id += 1
        result = self.command_id
        return result

    def _rpc_send(self, cmd_data, cmd_name, command_id=None, has_next=False) -> None:
        flipper_message = flipper_pb2.Main()
        flipper_message.command_id = command_id
        if not command_id:
            flipper_message.command_id = self.get_command_id()
        flipper_message.has_next = has_next

        flipper_message.command_status = flipper_pb2.CommandStatus.Value("OK")
        getattr(flipper_message, cmd_name).CopyFrom(cmd_data)
        data = bytearray(_VarintBytes(flipper_message.ByteSize()) + flipper_message.SerializeToString())

        self._serial.write(data)

    def _rpc_read_answer(self):
        while True:
            data = self._rpc_read_any()
            if data.command_id == self.command_id:
                break
        return data

    def _rpc_send_and_read_answer(self, cmd_data, cmd_name, has_next=False):
        self._rpc_send(cmd_data, cmd_name, has_next)
        return self._rpc_read_answer()

    def _rpc_read_any(self):
        length = self._read_varint_32()
        data = flipper_pb2.Main()
        data.ParseFromString(self._serial.read(size=length))
        return data

    def get_files(self, path="/ext") -> list:
        storage_response = []
        cmd_data = storage_pb2.ListRequest()

        cmd_data.path = path
        rep_data = self._rpc_send_and_read_answer(cmd_data, "storage_list_request")

        storage_response.extend(
            MessageToDict(message=rep_data.storage_list_response, including_default_value_fields=True, )["file"])

        while rep_data.has_next:
            rep_data = self._rpc_read_answer()
            storage_response.extend(
                MessageToDict(message=rep_data.storage_list_response, including_default_value_fields=True, )["file"])

        return storage_response

    def file_read(self, path=None):
        storage_response = []
        cmd_data = storage_pb2.ReadRequest()
        cmd_data.path = path

        rep_data = self._rpc_send_and_read_answer(cmd_data, "storage_read_request")

        storage_response.append(rep_data.storage_read_response.file.data)

        while rep_data.has_next:
            rep_data = self._rpc_read_answer()
            storage_response.append(rep_data.storage_read_response.file.data)

        return b"".join(storage_response)

    def file_write(self, path=None, data=""):
        cmd_data = storage_pb2.WriteRequest()
        cmd_data.path = path

        if isinstance(data, str):
            data = data.encode()

        chunk_size = 512
        data_len = len(data)
        command_id = self.get_command_id()
        for chunk in range(0, data_len, chunk_size):

            chunk_data = data[chunk: chunk + chunk_size]

            cmd_data.file.data = chunk_data

            if (chunk + chunk_size) < data_len:
                self._rpc_send(cmd_data, "storage_write_request", has_next=True, command_id=command_id, )
            else:
                self._rpc_send(cmd_data, "storage_write_request", has_next=False, command_id=command_id, )
                break

        self._rpc_read_answer()

    def file_delete(self, path=None):
        cmd_data = storage_pb2.DeleteRequest()
        cmd_data.path = path
        cmd_data.recursive = True

        self._rpc_send_and_read_answer(cmd_data, "storage_delete_request")

    def mkdir(self, path="/ext"):
        cmd_data = storage_pb2.MkdirRequest()
        cmd_data.path = path

        rep_data = self._rpc_send_and_read_answer(cmd_data, "storage_mkdir_request")

    def file_rename(self, old, new):
        cmd_data = storage_pb2.RenameRequest()
        cmd_data.old_path = old
        cmd_data.new_path = new

        self._rpc_send_and_read_answer(cmd_data, "storage_rename_request")
