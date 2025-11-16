# imgcut

## 📌 About

**imgcut** is a lightweight and fast command-line utility that converts
all images in a selected folder to a **512×512** format.

The program automatically detects the best way to transform each image:

-   **If the image is larger than 512×512** --- it is
    **center-cropped**.\
-   **If the image is smaller** --- it is **expanded with white
    padding** to reach the required size.

All operations are fully multithreaded for maximum performance.

------------------------------------------------------------------------

## 📌 Usage

Launch the program using command-line parameters:

``` bash
imgcut --src <path_to_source_folder> --dst <path_to_destination_folder>
```

All supported images from the source folder will be loaded, processed
(cropped or expanded), and saved to the destination folder.

------------------------------------------------------------------------

## 📌 Multithreading

imgcut supports parallel processing.\
To specify the number of worker threads:

``` bash
imgcut --src <src> --dst <dst> --threads 24
```

If you don't specify the `--threads` option, the program automatically
selects the **optimal number of threads** based on your system.
