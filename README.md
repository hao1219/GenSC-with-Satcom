# GenSC-with-Satcom

We propose a **Generative Semantic Communication (GenSC)** system integrated with **Satellite Communication**, designed to recover semantic information degraded by channel impairments such as fading, noise, and delays.  
The system utilizes a **BART-like model** for semantic encoding and decoding, combined with token-level relationships to reconstruct or replace corrupted words during transmission over satellite channels.

---

## **Overview**

### Generative Semantic Communication with Satellite Links  
This system combines generative semantic communication with satellite communication links to provide robust sentence reconstruction capabilities:  

1. **Semantic Encoder**:  
   Encodes sentences into semantic representations, leveraging **token-level correlations** between consecutive words.  

2. **Channel Encoder/Decoder**:  
   Encodes the semantic representation into channel signals and ensures recovery after transmission through **satellite communication channels**, which may introduce fading, noise, and propagation delays.  

3. **Semantic Decoder**:  
   Decodes the received signals and recovers missing or corrupted words, reconstructing a coherent sentence that closely matches the original.  

---

## **System Pipeline**

1. **End User to User Terminal (UT)**  
   - The original sentence is transmitted to the User Terminal (UT) via terrestrial communication links.  

2. **UT to Satellite to Ground Gateway (GW)**  
   - The semantic representation passes through a satellite communication channel.  
   - Impairments such as **fading**, **beam interference**, and **propagation delays** are addressed during this phase.  

3. **Ground Gateway (GW) to End User**  
   - The signal is decoded and forwarded to the end user via ground links.  
   - The semantic decoder recovers the transmitted sentence.

---

## **Key Advantages**

- **Robust Against Channel Noise**:  
   The system effectively mitigates channel impairments such as fading and Gaussian noise.  

- **Token-Level Recovery**:  
   Utilizes token-level relationships to reconstruct missing or corrupted words.  

- **Satellite Integration**:  
   Supports long-distance communication through satellite links while preserving semantic meaning.  

---

## **Training Requirements**

- **Anaconda**  
- **Python 3.9**  
- **Git**

---
## **Setup SNS3 for satellite channel simulation**
1. **Install SNS3**  
- Follow the instructions provided in the official [SNS3 Satellite repository](https://github.com/sns3/sns3-satellite).  
- Use the **BAKE** method (the first method described in the repository) to install SNS3 successfully.
2. **Replace the `sat-cbr-example.cc` File**  
In the repository, there is an updated file named **`sat-cbr-example.cc`**. Replace the existing file in the **SNS3** directory with your version:

3. **File Path in SNS3 Repository:**
```bash
~/bake/source/ns-3.37/contrib/satellite/examples/sat-cbr-example.cc
```
```bash
cp /path/to/your/repository/sat-cbr-example.cc ~/bake/source/ns-3.37/contrib/satellite/examples/sat-cbr-example.cc
```





## **Clone the Repository**

To download the project:

```bash
git https://github.com/hao1219/GenSC-with-Satcom.git
cd  GenSC-with-Satcom
```
Creating Conda enviroment:
```bash
conda env create -f environment.yml
```
Run Script:
- note that the `GenSC-with-Satcom` directory should be parallel to the `bake` directory
```bash
python gensc_with_script.py
```


