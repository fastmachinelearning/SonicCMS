import pickle

domain_name = "mit"

with open(domain_name+"_result.txt") as f:
    content = f.readlines()
# you may also want to remove whitespace characters like `\n` at the end of each line
content = [x.strip() for x in content]

output_time = []
remote_time = []
client_time = []

for line in content:
    this_line = line.split()
    if (len(this_line) == 0):
        continue
    if (this_line[0] == "Remote"):
        remote_time.append(int(this_line[-1]))
    if (this_line[0] == "Client"):
        client_time.append(int(this_line[-1]))
    if (this_line[0] == "Output"):
        output_time.append(int(this_line[-1]))
   
#print(output_time)
#print(remote_time)

#plt.figure()
#plt.xlabel("Test number")
#plt.ylabel("Time (usec)")
#plt.savefig("fnal_result.png")

result_dic = {}

result_dic["Remote"] = remote_time
result_dic["Output"] = output_time
result_dic["Client"] = client_time

with open(domain_name+'_result.pkl', 'wb') as handle:
    pickle.dump(result_dic, handle, protocol=pickle.HIGHEST_PROTOCOL)