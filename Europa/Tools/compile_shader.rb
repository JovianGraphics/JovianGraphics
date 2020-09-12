# ==============================================================================
# Europa Shader Compiler
# ==============================================================================

shader_file_name = ""
output_file_name = ""

macros = []

for arg in ARGV
  if arg[0..6] == "output="
    output_file_name = arg[7..-1]
  elsif arg[0..6] == "define="
    macros.push(arg[7..-1])
  else
    shader_file_name = arg
  end
end

if shader_file_name.empty?
  raise 'No input file specified'
end

if output_file_name.empty?
  output_file_name = File.basename(shader_file_name) + ".h"
end

puts "Compiling " + shader_file_name + " to " + output_file_name

require 'fileutils'
FileUtils.mkdir_p File.dirname(output_file_name)

# ==============================================================================

compiler_options = ""

if File.extname(shader_file_name) == '.frag'
  compiler_options += "-DFRAGMENT"
end

if File.extname(shader_file_name) == '.vert'
  compiler_options += "-DVERTEX"
end

if File.extname(shader_file_name) == '.vert'
  compiler_options += "-DVERTEX"
end

if File.extname(shader_file_name) == '.comp'
  compiler_options += "-DCOMPUTE"
end

macros.each do |d|
  compiler_options += " -D" + d + " "
end

require 'open3'

spv_struct, err, status = Open3.capture3("glslc -mfmt=c \"#{shader_file_name}\" --target-env=vulkan -O " + compiler_options + " -o -")

if status != 0
  puts err
  raise 'Shader compile failed'
end

# ==============================================================================

headers_file = ""

headers_file += "// Auto-generated by Europa Shader Compiler\n"
headers_file += "\n"

line_num = 0
File.open(shader_file_name).each do |line|
  headers_file += "// " + line_num.to_s + "\t " + line
  line_num += 1
end

headers_file += "\n\n"

headers_file += "static const uint32 shader_spv_" + File.basename(output_file_name).gsub('.', '_') + "[] = \n"
headers_file += spv_struct
headers_file += ";"

# ==============================================================================

f = File.open(output_file_name, "w")
f.puts headers_file
f.close

# ==============================================================================