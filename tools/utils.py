import typing as tp


__all__ = ['number_of_ands']


def number_of_ands(filename: str) -> tp.Optional[int]:
    """
    Calculates number of AND gates in a circuit located at `filename`.
    """
    _sz = 0
    try:
        with open(filename) as file:
            for line in file:
                if "AND" in line:
                    _sz += 1
    except FileNotFoundError:
        return None  # In case the file is not found
    return _sz
