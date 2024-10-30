#!/usr/bin/env python3
import logging
import click
from cnc import CNCDevice

logging.basicConfig(
    level=logging.INFO, format="[%(asctime)s] %(levelname)s - %(message)s"
)


@click.command()
@click.option("--ip", default="192.168.0.11", help="CNC Machine IP Address")
@click.option("--port", default=8193, help="CNC Machine Port Number")
def main(ip, port):
    try:
        with CNCDevice(ip, port) as cnc:
            try:
                id = cnc.read_id()
                logging.info(f"CNC Machine ID: {id}")
            except Exception as e:
                logging.error(f"Failed to read machine id: {e}")

            try:
                speed = cnc.read_speed()
                logging.info(f"CNC Machine Speed: {speed}")
            except Exception as e:
                logging.error(f"Failed to read speed: {e}")

            try:
                speeds = cnc.read_speeds(-1)  # 모든 스핀들의 속도를 읽어옴
                logging.info(f"CNC Machine Speeds: {speeds}")
            except Exception as e:
                logging.error(f"Failed to read speeds: {e}")

            try:
                feed_rate = cnc.read_feed_rate()
                logging.info(f"CNC Machine Feed Rate: {feed_rate}")
            except Exception as e:
                logging.error(f"Failed to read feed rate: {e}")

            try:
                feed_rate_speed = cnc.read_feed_rate_and_speed(
                    -1
                )  # 피드레이트와 스피들 속도를 읽어옴
                logging.info(f"CNC Machine Feed Rate and Speed: {feed_rate_speed}")
            except Exception as e:
                logging.error(f"Failed to read feed rate and speed: {e}")

            try:
                type = -1  # Read all data of modal G code
                block = 0  # Previous block
                gcode = cnc.read_gcode(type, block)
                logging.info(f"CNC Machine GCode: {gcode}")
            except Exception as e:
                logging.error(f"Failed to read gcode: {e}")

    except Exception as e:
        logging.error(f"Failed to connect cnc machine: {e}")
        raise click.ClickException(str(e))


if __name__ == "__main__":
    main()
