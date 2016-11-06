#!/bin/csh
set user = $1
set Expected = HelloFrom.rananthan6.euqderXPhcNHvhfciZMVdOmMhPLjQSOcPEhTNxg
set Actual = `./BreakRSA  596694146308867627 57535041791752883  344176447917788365 106136232166737020 67388946026326620 527411692038590290 512084345524808396 490500508213095560 306227776125393070 156982813449649090 245138097563700186 307523675709702382`
echo "expected is $Expected" 
echo "actal    is $Actual"
if ( "$Expected" == "$Actual" ) then
echo "Grade for user $user is 100"
else
echo "Grade for user $user is 50"
endif
