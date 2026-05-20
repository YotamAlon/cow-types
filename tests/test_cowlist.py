import pytest
from cow_types import cowlist


def test_init_empty():
    # Arrange / Act
    l = cowlist()

    # Assert
    assert len(l) == 0


def test_init_from_list():
    # Arrange / Act
    l = cowlist([1, 2, 3])

    # Assert
    assert l[0] == 1
    assert l[1] == 2
    assert l[2] == 3
    assert len(l) == 3


def test_init_from_iterable():
    # Arrange / Act
    l = cowlist(range(3))

    # Assert
    assert len(l) == 3
    assert l[0] == 0


def test_init_sharing_from_cowlist():
    # Arrange
    l = cowlist([1, 2])

    # Act  — shares underlying list without copying
    m = cowlist(l)

    # Assert
    assert m[0] == 1


def test_setitem_creates_new_instance():
    # Arrange
    l = cowlist([1, 2, 3])
    original_id = id(l)

    # Act
    l[1] = 99

    # Assert
    assert id(l) != original_id
    assert l[1] == 99


def test_setitem_does_not_mutate_shared_data():
    # Arrange
    l = cowlist([1, 2, 3])
    m = cowlist(l)

    # Act
    l[0] = 99

    # Assert
    assert m[0] == 1


def test_delitem_creates_new_instance():
    # Arrange
    l = cowlist([1, 2, 3])
    original_id = id(l)

    # Act
    del l[1]

    # Assert
    assert id(l) != original_id
    assert list(l) == [1, 3]


def test_delitem_out_of_range_raises():
    # Arrange
    l = cowlist([1, 2])

    # Act / Assert
    with pytest.raises(IndexError):
        del l[10]


def test_getitem_slice():
    # Arrange
    l = cowlist([1, 2, 3, 4, 5])

    # Assert
    assert l[1:3] == [2, 3]


def test_setitem_slice_creates_new_instance():
    # Arrange
    l = cowlist([1, 2, 3])
    original_id = id(l)

    # Act
    l[0:2] = [10, 20]

    # Assert
    assert id(l) != original_id
    assert list(l) == [10, 20, 3]


def test_append_creates_new_instance():
    # Arrange
    l = cowlist([1, 2])
    original_id = id(l)

    # Act
    l.append(3)

    # Assert
    assert id(l) != original_id
    assert l[2] == 3


def test_append_does_not_mutate_shared_data():
    # Arrange
    l = cowlist([1, 2])
    m = cowlist(l)

    # Act
    l.append(3)

    # Assert
    assert len(m) == 2


def test_extend_creates_new_instance():
    # Arrange
    l = cowlist([1, 2])
    original_id = id(l)

    # Act
    l.extend([3, 4])

    # Assert
    assert id(l) != original_id
    assert list(l) == [1, 2, 3, 4]


def test_insert_creates_new_instance():
    # Arrange
    l = cowlist([1, 3])
    original_id = id(l)

    # Act
    l.insert(1, 2)

    # Assert
    assert id(l) != original_id
    assert list(l) == [1, 2, 3]


def test_remove_creates_new_instance():
    # Arrange
    l = cowlist([1, 2, 3])
    original_id = id(l)

    # Act
    l.remove(2)

    # Assert
    assert id(l) != original_id
    assert list(l) == [1, 3]


def test_remove_missing_raises():
    # Arrange
    l = cowlist([1, 2])

    # Act / Assert
    with pytest.raises(ValueError):
        l.remove(99)


def test_pop_returns_value_and_creates_new_instance():
    # Arrange
    l = cowlist([1, 2, 3])
    original_id = id(l)

    # Act
    val = l.pop()

    # Assert
    assert val == 3
    assert id(l) != original_id
    assert list(l) == [1, 2]


def test_pop_with_index():
    # Arrange
    l = cowlist([1, 2, 3])

    # Act
    val = l.pop(0)

    # Assert
    assert val == 1
    assert list(l) == [2, 3]


def test_clear_creates_new_instance():
    # Arrange
    l = cowlist([1, 2, 3])
    original_id = id(l)

    # Act
    l.clear()

    # Assert
    assert id(l) != original_id
    assert len(l) == 0


def test_sort_creates_new_instance():
    # Arrange
    l = cowlist([3, 1, 2])
    original_id = id(l)

    # Act
    l.sort()

    # Assert
    assert id(l) != original_id
    assert list(l) == [1, 2, 3]


def test_sort_reverse():
    # Arrange
    l = cowlist([1, 2, 3])

    # Act
    l.sort(reverse=True)

    # Assert
    assert list(l) == [3, 2, 1]


def test_reverse_creates_new_instance():
    # Arrange
    l = cowlist([1, 2, 3])
    original_id = id(l)

    # Act
    l.reverse()

    # Assert
    assert id(l) != original_id
    assert list(l) == [3, 2, 1]


def test_copy_returns_independent_cowlist():
    # Arrange
    l = cowlist([1, 2])

    # Act
    m = l.copy()
    l.append(3)

    # Assert
    assert isinstance(m, cowlist)
    assert len(m) == 2


def test_index():
    # Arrange
    l = cowlist([10, 20, 30])

    # Assert
    assert l.index(20) == 1


def test_index_missing_raises():
    # Arrange
    l = cowlist([1, 2])

    # Act / Assert
    with pytest.raises(ValueError):
        l.index(99)


def test_count():
    # Arrange
    l = cowlist([1, 2, 1, 3, 1])

    # Assert
    assert l.count(1) == 3
    assert l.count(2) == 1
    assert l.count(99) == 0


def test_contains():
    # Arrange
    l = cowlist([1, 2, 3])

    # Assert
    assert 2 in l
    assert 99 not in l


def test_iter():
    # Arrange
    l = cowlist([1, 2, 3])

    # Assert
    assert list(l) == [1, 2, 3]


def test_equality_with_list():
    # Arrange
    l = cowlist([1, 2, 3])

    # Assert
    assert l == [1, 2, 3]
    assert l != [1, 2]


def test_equality_with_cowlist():
    # Arrange
    l = cowlist([1, 2])
    m = cowlist([1, 2])

    # Assert
    assert l == m


def test_repr():
    # Arrange
    l = cowlist([1, 2, 3])

    # Assert
    assert repr(l) == "cowlist([1, 2, 3])"


def test_multiple_mutations_each_create_new_instance():
    # Arrange
    l = cowlist([1])
    id1 = id(l)

    # Act
    l.append(2)
    id2 = id(l)
    l.append(3)
    id3 = id(l)

    # Assert
    assert id1 != id2
    assert id2 != id3
    assert list(l) == [1, 2, 3]
