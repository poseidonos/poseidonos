from abc import *


class CliInterface(metaclass=ABCMeta):
    @abstractmethod
    def _send_cli(self):
        pass

    @abstractmethod
    def array_add_spare(self, arr_name, dev_name):
        pass

    @abstractmethod
    def array_create(self, buffer_dev, user_devs, spare_devs, arr_name, raid_type):
        pass

    @abstractmethod
    def array_list(self, arr_name):
        pass

    @abstractmethod
    def array_mount(self, arr_name, wb_mode):
        pass

    @abstractmethod
    def array_reset(self):
        pass

    @abstractmethod
    def array_unmount(self, arr_name):
        pass

    @abstractmethod
    def device_create(self, dev_name, dev_type, num_blk, blk_size, numa):
        pass

    @abstractmethod
    def device_list(self):
        pass

    @abstractmethod
    def device_scan(self):
        pass

    @abstractmethod
    def logger_set_level(self, level):
        pass

    @abstractmethod
    def qos_create(self, arr_name, vol_name, maxbw, maxiops, minbw, miniops):
        pass

    @abstractmethod
    def qos_reset(self, arr_name, vol_name):
        pass

    @abstractmethod
    def subsystem_add_listener(self, nqn, trtype, target_ip, port):
        pass

    @abstractmethod
    def subsystem_create(self, nqn, sn):
        pass

    @abstractmethod
    def subsystem_create_transport(self, trtype, num_shared_buf):
        pass

    @abstractmethod
    def subsystem_list(self):
        pass

    @abstractmethod
    def system_set_property(self, impact):
        pass

    @abstractmethod
    def system_stop(self):
        pass

    @abstractmethod
    def telemetry_start(self):
        pass

    @abstractmethod
    def telemetry_stop(self):
        pass

    @abstractmethod
    def volume_create(self, vol_name, vol_size, arr_name, maxiops, maxbw):
        pass

    @abstractmethod
    def volume_mount(self, vol_name, subnqn, arr_name):
        pass
