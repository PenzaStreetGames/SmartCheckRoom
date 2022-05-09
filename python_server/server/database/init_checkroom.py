from json import loads, dumps
from repositories import *


def init_checkroom():
    config_file = "checkroom_struct.json"

    with open(config_file, "r", encoding="utf-8") as infile:
        struct = loads(infile.read())
    print(struct)

    if not struct["init"]:
        control_queue_id = 1

        for control_box_info in struct["control_boxes"]:
            control_id = control_box_info["id"]
            nfc_info = control_box_info["nfc"]
            hangers_info = control_box_info["hangers"]
            control_box = ControlBoxRepository.create(control_id)

            queue_in, queue_out = control_box_info["queue"]["in"], control_box_info["queue"]["out"]
            for i in range(1, queue_in + 1):
                control_queue = ControlQueueRepository.create(id=control_queue_id, direction="in", position=i)
                control_queue.control_box = control_box
                control_queue_id += 1
            for i in range(1, queue_out + 1):
                control_queue = ControlQueueRepository.create(id=control_queue_id, direction="out", position=i)
                control_queue.control_box = control_box
                control_queue_id += 1

            nfc_id = nfc_info["id"]
            nfc = NfcRepository.create(id=nfc_id, control_id=control_id)
            nfc.control_box = control_box

            for hanger_info in hangers_info:
                hanger_id = hanger_info["id"]

                hanger = HangerRepository.create(id=hanger_id, control_id=control_id)
                hanger.control_box = control_box

                hooks_info = hanger_info["hooks"]
                hooks_from = hooks_info["from"]
                hooks_to = hooks_info["to"]
                for i in range(hooks_from, hooks_to + 1):
                    hook = HookRepository.create(id=i, hanger_id=hanger_id)
                    hook.hanger = hanger

        session.commit()
        struct["init"] = True

    with open(config_file, "w", encoding="utf-8") as outfile:
        outfile.write(dumps(struct, indent=2))


if __name__ == '__main__':
    init_checkroom()
