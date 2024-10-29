#!/usr/bin/env python3
from fwlib import Context
import logging
import click

logging.basicConfig(
    level=logging.INFO, format="[%(asctime)s] %(levelname)s - %(message)s"
)


class CNCDevice:
    def __init__(self, ip="192.168.0.11", port=8193) -> None:
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


@click.command()
@click.option("--ip", default="192.168.0.11", help="CNC Machine IP Address")
@click.option("--port", default=8193, help="CNC Machine Port Number")
def main(ip, port):
    with CNCDevice(ip, port) as cnc:
        try:
            id = cnc.read_id()
            logging.info(f"CNC Machine ID: {id}")
        except Exception as e:
            logging.error(f"Error: {e}")

        try:
            speed = cnc.read_speed()
            logging.info(f"CNC Machine Speed: {speed}")
        except Exception as e:
            logging.error(f"Error: {e}")

        try:
            speeds = cnc.read_speeds(-1)  # 모든 스핀들의 속도를 읽어옴
            logging.info(f"CNC Machine Speeds: {speeds}")
        except Exception as e:
            logging.error(f"Error: {e}")

        try:
            feed_rate = cnc.read_feed_rate()
            logging.info(f"CNC Machine Feed Rate: {feed_rate}")
        except Exception as e:
            logging.error(f"Error: {e}")


if __name__ == "__main__":
    main()
