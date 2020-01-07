-- sample_wu.lua - no imperatives

batch = 172
subtype = "xyz"
param1="x"
param2="y"

-- workunit with hook function

batch = 173
function simrun()
  print("sth")
end

-- self driving workunit

batch = 174
boinc.appinit()
print("sth")
