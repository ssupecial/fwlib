#!/usr/bin/env python3
import logging
import click
from cnc import CNCDevice
import paho.mqtt.client as mqtt
import time
import json
import datetime as dt
from zoneinfo import ZoneInfo


logging.basicConfig(
    level=logging.INFO, format="[%(asctime)s] %(levelname)s - %(message)s"
)


def setup_mqtt(name, mqtt_ip, mqtt_port) -> mqtt.Client:
    mqtt_client = mqtt.Client(name)

    def on_connect(client, userdata, flags, rc):
        if rc == 0:
            logging.info("Connected to MQTT Broker!")
        else:
            logging.error("Failed to connect to MQTT Broker...")

    def on_disconnect(client, userdata, rc):
        if rc != 0:
            logging.error("Unexpected disconnection from MQTT Broker...")
        else:
            logging.info("Disconnected from MQTT Broker!")

    mqtt_client.on_connect = on_connect
    mqtt_client.on_disconnect = on_disconnect

    mqtt_client.connect(mqtt_ip, mqtt_port)
    mqtt_client.loop_start()

    return mqtt_client


@click.command()
@click.option("--ip", default="192.168.0.11", help="CNC Machine IP Address")
@click.option("--port", type=int, default=8193, help="CNC Machine Port Number")
@click.option("--mqtt_ip", help="MQTT Broker IP Address", required=True)
@click.option("--mqtt_port", type=int, default=1883, help="MQTT Broker Port Number")
@click.option("--mqtt_topic", default="cnc/controller", help="MQTT Topic")
@click.option(
    "--time_interval",
    type=float,
    default=1.0,
    help="Time Interval to read data(seconds)",
)
@click.option("--print_log", is_flag=True, default=False, help="Print log to console")
def main(ip, port, mqtt_ip, mqtt_port, mqtt_topic, time_interval, print_log):
    if print_log:
        logging.getLogger().setLevel(logging.DEBUG)
    else:
        logging.getLogger().setLevel(logging.ERROR)

    # MQTT Broker에 연결
    try:
        mqtt_client = setup_mqtt("CNC Machine", mqtt_ip, mqtt_port)
    except Exception as e:
        logging.error(f"Failed to connect to MQTT Broker: {e}")
        raise click.ClickException(str(e))

    # CNC Machine에 연결 후, time interval 주기마다 데이터 읽어옴
    try:
        start_time = time.perf_counter()
        with CNCDevice(ip, port) as cnc:
            while True:
                message = {}
                try:
                    # 머신 ID 읽어옴
                    id = cnc.read_id()
                    message["id"] = id
                    logging.info(f"[CNC Machine ID]\n{id}")
                except Exception as e:
                    logging.error(f"Failed to read machine id: {e}")

                try:
                    # 스핀들 속도 하나 읽어옴
                    speed = cnc.read_speed()
                    message["speed"] = speed
                    logging.info(f"[CNC Machine Speed]\n{speed}")
                except Exception as e:
                    logging.error(f"Failed to read speed: {e}")

                try:
                    # 모든 스핀들의 속도를 읽어옴
                    speeds = cnc.read_speeds(-1)
                    message["speeds"] = speeds
                    logging.info(f"[CNC Machine Speeds]\n{speeds}")
                except Exception as e:
                    logging.error(f"Failed to read speeds: {e}")

                try:
                    # 이송 속도 읽어옴
                    feed_rate = cnc.read_feed_rate()
                    message["feed_rate"] = feed_rate
                    logging.info(f"[CNC Machine Feed Rate]\n{feed_rate}")
                except Exception as e:
                    logging.error(f"Failed to read feed rate: {e}")

                try:
                    # 이송 속도와 스핀들 속도를 읽어옴
                    feed_rate_speed = cnc.read_feed_rate_and_speed(-1)
                    message["feed_rate_speed"] = feed_rate_speed
                    logging.info(
                        f"[CNC Machine Feed Rate and Speed]\n{feed_rate_speed}"
                    )
                except Exception as e:
                    logging.error(f"Failed to read feed rate and speed: {e}")

                """cnc_rdgcode"""
                try:
                    # 현재 활성화된 블럭의 modal Gcode를 한 번에 읽어옴
                    gcode = cnc.read_all_active_modal_g_codes()
                    message["modal_gcode"] = gcode
                    logging.info(f"[CNC Machine Modal GCode]\n{gcode}")
                except Exception as e:
                    logging.error(f"Failed to read modal gcode: {e}")

                try:
                    # 현재 활성화된 블럭의 one shot Gcode를 한 번에 읽어옴
                    gcode = cnc.read_all_active_one_shot_g_codes()
                    message["one_shot_gcode"] = gcode
                    logging.info(f"[CNC Machine One shot GCode]\n{gcode}")
                except Exception as e:
                    logging.error(f"Failed to read one shot gcode: {e}")

                """cnc_modal"""
                try:
                    # 현재 활성화된 블럭의 modal data를 한 번에 읽어옴
                    modal_data = cnc.read_all_active_modal_data()
                    message["modal_data"] = modal_data
                    logging.info(f"[CNC Machine Modal Data]\n{modal_data}")
                except Exception as e:
                    logging.error(f"Failed to read modal data: {e}")

                try:
                    # 현재 활성화된 블럭의 1 shot data를 한 번에 읽어옴
                    one_shot_data = cnc.read_all_active_one_shot_data()
                    message["one_shot_data"] = one_shot_data
                    logging.info(f"[CNC Machine One shot Data]\n{one_shot_data}")
                except Exception as e:
                    logging.error(f"Failed to read one shot data: {e}")

                try:
                    # 현재 활성화된 블럭의 axis data를 한 번에 읽어옴
                    axis_data = cnc.read_all_active_axis_data()
                    message["axis_data"] = axis_data
                    logging.info(f"[CNC Machine Axis Data]\n{axis_data}")
                except Exception as e:
                    logging.error(f"Failed to read axis data: {e}")

                try:
                    # 현재 활성화된 블럭의 Gcode 외 data를 한 번에 읽어옴
                    other_data = cnc.read_all_active_other_data()
                    message["other_data"] = other_data
                    logging.info(f"[CNC Machine Other Data]\n{other_data}")
                except Exception as e:
                    logging.error(f"Failed to read other data: {e}")

                timestamp = dt.datetime.now(ZoneInfo("Asia/Seoul")).strftime(
                    "%Y-%m-%d %H:%M:%S"
                )
                message["timestamp"] = timestamp
                end_time = time.perf_counter()
                remaining_time = time_interval - (end_time - start_time)
                if remaining_time > 0:
                    time.sleep(remaining_time)
                mqtt_client.publish(mqtt_topic, json.dumps(message))
                start_time = time.perf_counter()

    except Exception as e:
        logging.error(f"Failed to connect cnc machine: {e}")
        raise click.ClickException(str(e))


if __name__ == "__main__":
    main()
