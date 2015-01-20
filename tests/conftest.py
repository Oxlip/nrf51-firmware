import pytest

def pytest_addoption(parser):
    parser.addoption('--macaddr', action='store', default='E8:E8:1C:9C:1F:D9',
                     help="BLE MAC Address of the device")
    parser.addoption('--count', default=1, type='int', metavar='count',
                     help='Run each test the specified number of times')

@pytest.fixture
def mac_address(request):
    return request.config.getoption("--macaddr")


def pytest_generate_tests (metafunc):
    for i in range (metafunc.config.option.count):
        metafunc.addcall()

