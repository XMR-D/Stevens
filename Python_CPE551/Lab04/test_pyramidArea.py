# Author: Guillaume Wantiez
# Date: 02/16/2026
# Description: Testing suite for pyramidArea.py


import pytest
import pyramidArea as pA


def test_calcBaseArea_regular():
    result = pA.calcBaseArea(15)
    assert result == 225

@pytest.mark.xfail(reason="Testing argument is not a proper typre (string)")
def test_calcBaseArea_string():
    pA.calcBaseArea("5") 

def test_calcSideArea_regular():
    result = pA.calcSideArea(side=15, height=5)
    assert (result >= 270.41 and result <= 270.42)

def test_calcSideArea_rounded():
    result = pA.calcSideArea(side=10, height=3)
    assert round(result, 2) == 116.62

@pytest.mark.skip(reason="Function print a string")
def test_prntSurfArea():
    assert pA.prntSurfArea(15, 10)