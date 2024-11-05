from fwlib import Context


class CNCDevice:
    def __init__(self, ip="192.168.0.11", port=8193):
        self.ip = ip
        self.port = port
        self.context = None

    def __enter__(self):
        self.context = Context(host=self.ip, port=self.port)
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        if self.context:
            self.context.__exit__(exc_type, exc_val, exc_tb)
            self.context = None

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

    """Read Gcode"""

    def _read_gcode(self, type, block):
        """
        Read Gcode data.

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

    def read_all_previous_modal_g_codes(self):
        # Read all previous data of modal G code at a time.
        return self._read_gcode(-1, 0)

    def read_all_active_modal_g_codes(self):
        # Read all active data of modal G code at a time.
        return self._read_gcode(-1, 1)

    def read_all_next_modal_g_codes(self):
        # Read all next data of modal G code at a time.
        return self._read_gcode(-1, 2)

    def read_all_previous_one_shot_g_codes(self):
        # Read all previous data of one shot G code at a time.
        return self._read_gcode(-2, 0)

    def read_all_active_one_shot_g_codes(self):
        # Read all active data of one shot G code at a time.
        return self._read_gcode(-2, 1)

    def read_all_next_one_shot_g_codes(self):
        # Read all next data of one shot G code at a time.
        return self._read_gcode(-2, 2)

    """Read Modal"""

    def _read_modal(self, type, block):
        """
        Read modal data.

        Args:
            type (int, required): Group of Gcode
                2) Series 30i, 0i-D/F, PMi-A
                    -4: Read all data of 1 shot G code at a time.
                    -3: Read all data concerning axis other than G code at a time. (Only the axis number in a path is 8 axes or less.)
                    -2: Read all data other than G code at a time.
                    -1: Read all data of G code at a time. (type= 0 to 20)
                    0 to 20: Read data of G code one by one.
                    100 to 126: Read data other than G code one by one.
                    200 to 207: Read the data concerning axis other than G code one by one.
                    300: Read the data of 1 shot G code one by one.
            block (int, required): block number
                0: previous block
                1: active block
                2: next block

        Returns:
            Dict: Dictionary of modal data.
                {
                    'datano': int,    # Kind of modal data
                    'type': int,      # Objective block

                    '''Union'''
                    1)
                    'g_data': {
                        'code': int,  # Gcode
                        'commanded': int,  # Commanded value
                    }
                    2)
                    'g_rdata': [      # Gcode datas (Length: 21)
                        {
                            'code': int,  # Gcode
                            'commanded': int,  # Commanded value
                        },
                        ...
                    ]
                    3)
                    'g_1shot': [      # Gcode datas (Length: 4)
                        {
                            'code': int,  # Gcode
                            'commanded': int,  # Commanded value
                        },
                        ...
                    ]
                    4)
                    'aux': {
                        'aux_data': int,
                        'flag1': str,
                        'flag2': str,
                    }
                    5)
                    'raux1': [        # Auxiliary datas (Length: 27)
                        {
                            'aux_data': int,
                            'flag1': str,
                            'flag2': str,
                        },
                        ...
                    ]
                    6)
                    'raux2': [        # Auxiliary datas (Length: 8)
                        {
                            'aux_data': int,
                            'flag1': str,
                            'flag2': str,
                        },
                        ...
                    ]
        """
        return self.context.rdmodal(type=type, block=block)

    def read_all_previous_modal_data(self):
        # Read all previous data of modal Gcode at a time.
        return self._read_modal(-1, 0)

    def read_all_active_modal_data(self):
        # Read all active data of modal Gcode at a time.
        return self._read_modal(-1, 1)

    def read_all_next_modal_data(self):
        # Read all next data of modal Gcode at a time.
        return self._read_modal(-1, 2)

    def read_all_previous_one_shot_data(self):
        # Read all previous data of 1 shot Gcode at a time.
        return self._read_modal(-4, 0)

    def read_all_active_one_shot_data(self):
        # Read all active data of 1 shot Gcode at a time.
        return self._read_modal(-4, 1)

    def read_all_next_one_shot_data(self):
        # Read all next data of 1 shot G code at a time.
        return self._read_modal(-4, 2)

    def read_all_previous_axis_data(self):
        # Read all previous data concerning axis other than G code at a time.
        return self._read_modal(-3, 0)

    def read_all_active_axis_data(self):
        # Read all active data concerning axis other than G code at a time.
        return self._read_modal(-3, 1)

    def read_all_next_axis_data(self):
        # Read all next data concerning axis other than G code at a time.
        return self._read_modal(-3, 2)

    def read_all_previous_other_data(self):
        # Read all previous data other than G code at a time.
        return self._read_modal(-2, 0)

    def read_all_active_other_data(self):
        # Read all active data other than G code at a time.
        return self._read_modal(-2, 1)

    def read_all_next_other_data(self):
        # Read all next data other than G code at a time.
        return self._read_modal(-2, 2)
