from fwlib import Context
import logging

logging.basicConfig(
    level=logging.INFO, format="[%(asctime)s] %(levelname)s - %(message)s"
)


class CNC_Device:
    def __init__(self, ip="192.168.0.11", port=8193) -> None:
        self.ip = ip
        self.port = port
        self.device = Context(host=ip, port=port)


if __name__ == "__main__":
    cnc = CNC_Device()
    try:
        id = cnc.read_id()
        logging.info(f"CNC Machine ID: {id}")
    except Exception as e:
        logging.error(f"Error: {e}")

    try:
        speed = cnc.acts()
        logging.info(f"CNC Machine Speed: {speed}")
    except Exception as e:
        logging.error(f"Error: {e}")

    try:
        speeds = cnc.acts2()
        logging.info(f"CNC Machine Speeds: {speeds}")
    except Exception as e:
        logging.error(f"Error: {e}")

    try:
        feed_rate = cnc.actf()
        logging.info(f"CNC Machine Feed Rate: {feed_rate}")
    except Exception as e:
        logging.error(f"Error: {e}")
