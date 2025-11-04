# aluan-CART-project_ucsc-ece129
 
>⚠️ Note: This README belongs to the aluan-CART-project_ucsc-ece129
 repository.
Python scripts and windows powershell scripts that automatically generate MLA/APA reports and essays for most classes. 

----
----
## C.A.R.T. - Carry Assist Robotic Transport
### ECE 129A CAPSTONE Project Proposal (Fall 2025)

  Project 10  
  C.A.R.T. - Carry Assist Robotic Transport (Robotic_Cart_presented-draft.pdf)  
  Project Brief: [Carry_Assist_Robotic_Transport__ProjectBrief.pdf](docs/Carry_Assist_Robotic_Transport__ProjectBrief.pdf)  
  Presentation (presented in class): [Robotic_Cart_presented-draft.pdf](Robotic_Cart_presented-draft.pdf)  
  Presentation (revised draft): [Carry_Assist_Robotic_Transport__Slideshow.pdf](slides/Carry_Assist_Robotic_Transport__Slideshow.pdf)

<img src="assets/CART__ProjectBrief_thmbnl.png" width="385px" align="center">

<br><br>
<b><u>Brief Summary of C.A.R.T</u></b>
<br>

```[Problem]```
Many individuals, including delivery workers, university students, and people with physical disabilities, face significant challenges in reaching various locations due to limited accessibility and the high cost of infrastructure.

```[Stakeholders_and_Why_It_Matters]``` 
Current solutions often rely on cars and trucks, which are inefficient, time-consuming, and impractical for off-road or uneven terrains, especially where parking availability is inconsistent.

```[Focus_and_Objective]``` 
To address these challenges, a semi-autonomous robotic cart is proposed to assist individuals in carrying their belongings efficiently and affordably. The embedded system will provide a reliable means of transport across diverse environments while reducing the physical burden of carrying loads.

```[How]```
The mechanical and electrical design of the cart will incorporate low-cost motors and materials in order to make it affordable.

<br>

----

### <b><u>Team Meeting Details</u></b>

[when2meet-team-availability](ttps://www.when2meet.com/?33322720-xyuxH) (set up by ```James Tse```)

#### <b>First Meeting:</b>
- ```Date & Time:``` Wednesday, November 5 @3:30 – 4:30pm
- ```Final Location:``` Science and Engineering Library, Room 330
- ```Meeting Notes``` [CART First Project Meeting Notes](https://docs.google.com/document/d/1Mm5sm0tWaWyfTjY2sSdBk_tzSK5iY-w6AVKgav2Nx5k/edit?tab=t.0)

----

If you are.... 
- a classmate of mine in **S.C. Petersen**'s class
- part of a **STEM-affiliated club**
  - Slugbotics
  - Rocket Team
  - etc.
- a professor from the **University of California, Santa Cruz** 
- interested in *becoming a stakeholder* for this project pitch idea

and are interested in reaching out to me and add yourselves to my stakeholders list, email me for further interest;

```Email:``` aluan@ucsc.edu   
```Resume:``` [Audrey Luan's Resume](aluan_RESUME.md)

<img src="assets/robotic-cart.png" width="385px" align="center">

----

https://robotics.omron.com/products/mobile-robots/ld-series/ 

----
----

## Important Notes & Updates

#### 1. Repository being renamed

The Github repository <span style="color:#2196f3; background-color:#f3e5f5; padding:2px 6px; border-radius:4px;">aluan-report_temp_generator</span> was renamed to <b><span style="color:#DAA520; background-color:#f3e5f5; padding:2px 6px; border-radius:4px;">aluan-CART-project_ucsc-ece129</span></b> due to the CAPSTONE project proposal in the <span style="color:#DAA520;">University of California, Santa Cruz</span>.

The remote link remains unchanged to maintain access for collaborators who are still using the existing repository URL.

```bash
$ git config --get remote.origin.url
git@github.com:aluan-fooshe/aluan-report_temp_generator.git

$ mv report_template_generator aluan-CART-project_ucsc-ece129

$ git remote -v
origin  git@github.com:aluan-fooshe/aluan-report_temp_generator.git (fetch)
origin  git@github.com:aluan-fooshe/aluan-report_temp_generator.git (push)

```

#### 2. Incorrect documentation for another repository

The contents that are listed here below was misplaced in the ```aluan-daily_log_entries/README.md```!

- [aluan-daily_log_entries README.md](https://github.com/aluan-fooshe/aluan-daily_log_entries/blob/main/README.md)
- [aluan-daily_log_entries repository](https://github.com/aluan-fooshe/aluan-daily_log_entries/)

#### 3. UTF-8 vs UTF-16

The encoding of the ".txt file" is very important to list in this function in Filelist_workbook.py;

```python
  def import_dictionary(self, filename):
      dictionary = {}
      item0 = []

      try:
          f = open(filename, 'r', encoding='utf-16')
          list = f.readlines()
          for item in list:
              item = item.strip('\n')
              item0.append(item)
```

**Problem:** The `.txt` file is saved in UTF-16 encoding, but Python's default file reading uses UTF-8 encoding. This mismatch causes garbled text with visible byte order marks (ÿþ) and extra spaces between characters.

**Solution:** Explicitly specify the encoding when opening the file:

```python
  f = open(filename, 'r', encoding='utf-16')
```

**Note:** Always match the encoding parameter to how the file was actually saved. Common Windows programs may save files in UTF-16, while most modern systems default to UTF-8.

**Documentation worth citing ↓**

---
4.7. UTF-8 mode
Added in version 3.7.

Windows still uses legacy encodings for the system encoding (the ANSI Code Page). Python uses it for the default encoding of text files (e.g. locale.getencoding()).

This may cause issues because UTF-8 is widely used on the internet and most Unix systems, including WSL (Windows Subsystem for Linux).

You can use the Python UTF-8 Mode to change the default text encoding to UTF-8. You can enable the Python UTF-8 Mode via the -X utf8 command line option, or the PYTHONUTF8=1 environment variable. See PYTHONUTF8 for enabling UTF-8 mode, and Python install manager for how to modify environment variables. [1]

---
Hello giriv_1210,

Good day! Thank you for bringing this to our Microsoft Community Forum.

I understand how frustrating it can be when the same .txt file behaves differently across different systems and versions of Outlook. Let’s work through this together.

The issue you’ve described seems to be related to how Outlook on Windows 11 interprets the encoding of .txt file attachments, defaulting to UTF-16 LE instead of UTF-8. This can cause the file to appear unreadable if the system or application isn’t set to interpret UTF-16 correctly. [2]

----
### References

[1] Python Software Foundation, "Built-in Functions", Python 3.14.0 Documentation. 
[Online]. Available: https://docs.python.org/3/library/functions.html#open. 
[Accessed: Oct. 25, 2025].

[2] Microsoft Ignite, "Same .txt file opens in utf-8 encoding in windows 10 outlook and utf-16 le unreadable format on windows 11 outlook", by Anonymous. 
[Online]. Available: https://learn.microsoft.com/en-us/answers/questions/4661269/same-txt-file-opens-in-utf-8-encoding-in-windows-1. 
[Accessed: Oct. 25, 2025].

[3] CodeTwo, "Non-Latin or accented characters are displayed incorrectly in emails". [Online]. Available: https://www.codetwo.com/kb/incorrect-characters-in-emails/

