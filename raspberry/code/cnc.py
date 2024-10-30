import logging
from fwlib import Context

logging.basicConfig(
    level=logging.INFO, format="[%(asctime)s] %(levelname)s - %(message)s"
)


class CNCDevice:
    def __init__(self, ip="192.168.0.11", port=8193):
        self.ip = ip
        self.port = port
        self.context = None

    def __enter__(self):
        try:
            self.context = Context(host=self.ip, port=self.port)
            return self
        except Exception as e:
            logging.error(f"Failed to connect to CNC Machine: {e}")
            raise

    def __exit__(self, exc_type, exc_val, exc_tb):
        if self.context:
            self.context.__exit__(exc_type, exc_val, exc_tb)

    def read_id(self):
        return self.context.read_id()

    def read_speed(self):
        return self.context.acts()

    def read_speeds(self, spindle_no=-1):
        return self.context.acts2(spindle_no)

    def read_feed_rate(self):
        return self.context.actf()

    def read_feed_rate_and_speed(self, type=-1):
        """Read CNC feed rate and spindle speed data.

        Args:
            type (int, optional): Type of data to read.
                0: Feed rate only
                1: Spindle speed only
                -1: Both feed rate and spindle speed. Defaults to -1.

        Returns:
            dict: Dictionary containing feed rate and/or spindle speed data.
                {
                    'feed_rate': {
                        'data': int,    # Actual feed rate value (raw)
                        'dec': int,     # Decimal point position
                        'unit': int,    # Unit type (0:mm/min, 1:inch/min, 2:rpm, 3:mm/rev, 4:inch/rev)
                        'reserve': int, # Reserved value
                        'name': str,    # Data identifier ('F')
                        'suff': str     # Suffix for identification
                    },
                    'spindle_speed': {
                        'data': int,    # Actual spindle speed value (raw)
                        'dec': int,     # Decimal point position
                        'unit': int,    # Unit type (2:rpm)
                        'reserve': int, # Reserved value
                        'name': str,    # Data identifier ('S')
                        'suff': str     # Spindle number in ASCII
                    }
                }

        Example:
            >>> data = cnc.read_feed_rate_and_speed()
            >>> feed_rate = data['feed_rate']['data'] / (10 ** data['feed_rate']['dec'])
            >>> spindle_speed = data['spindle_speed']['data'] / (10 ** data['spindle_speed']['dec'])
        """
        return self.context.rdspeed(type)

    def read_gcode(self, type, block):
        """Read Gcode data.

        Args:
            type (int, required): Group of Gcode
                1) Series 15i
                    0 to 18,24,25,27: Read data of modal G code one by one.
                    -1: Read all data of modal G code at a time.
                    100 to 103: Read data of one shot G code one by one.
                    -2: Read all data of one shot G code at a time.
                2) Series 30i, 0i-D/F, PMi-A
                    0 to 36: Read data of modal G code one by one.
                    -1: Read all data of modal G code at a time.
                    100 to 103: Read data of one shot G code one by one.
                    -2: Read all data of one shot G code at a time.
            block (int, required): block number
                0: previous block
                1: active block
                2: next block

        Returns:
            List: List of Gcode data.
                [
                    {
                        'group': int,    # Group number
                        'flag': int,     # Additional information
                        'code': str,     # Gcode (Length: 8)
                    },
                    ...
                ]
        """
        return self.context.rdgcode(type=type, block=block)
