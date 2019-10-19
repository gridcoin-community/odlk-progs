-- sample user override .lua

-- example of overriding compiler
compile_cpp11 = function(output, inputs)
  local input_args = inputs.join(" ")
  local cmd = "g++ --std=c++11 -O3 -o "..output.." "..input_args
  xx=os.exec(cmd)
end

-- original
function compile_cpp11 = function(output, inputs)
  compiler_detect()
  compile_xx(output, inputs)
end
