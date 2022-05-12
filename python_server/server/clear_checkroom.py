"""aboba"""

from server.database.repositories import *


def clear_checkroom():
    """
    Функция очистки гардероба от информации о метках

    Освобождает очереди на внос и на вынос

    Удаляет историю тегов

    Удаляет сами теги
    """
    control_queues = ControlQueueRepository.select_all()
    for control_queue in control_queues:
        control_queue.tag_id = None
    tag_states = TagStateRepository.select_all()
    DeleteService.delete_all(tag_states)
    tags = TagRepository.select_all()
    DeleteService.delete_all(tags)


if __name__ == '__main__':
    clear_checkroom()
