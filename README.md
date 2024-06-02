# CS202 Project Final Submission

## Project Description

**mgFS: Image-oriented File System**

### Overview

The mgFS project involves developing a large-scale program in C, focused on a system-level theme. This project aims to create a command-line utility for managing images within a specialized file system format, drawing inspiration from Facebook's "Haystack" system. The goal is to provide an efficient solution for handling large volumes of images, with automatic management of multiple resolutions and deduplication of identical images.

### Project Background

Social networks like Facebook handle hundreds of millions of images, presenting significant challenges for traditional file systems. These systems struggle with efficiency when managing vast numbers of files and are not designed to support multiple resolutions for each image. Inspired by the "Haystack" system, mgFS addresses these issues by consolidating multiple images and their various resolutions into a single file, containing both data and metadata.

### Key Features

1. **Reduced File Management Overhead**: By storing multiple images in a single file, the operating system manages fewer files, enhancing efficiency.
2. **Automatic Multi-Resolution Management**: The system automatically handles multiple resolutions (e.g., icon, preview, original) for each image.
3. **Image Deduplication**: Using the SHA-256 hash function, mgFS eliminates duplicate images by comparing their hash values, ensuring unique storage.

### Project Goals

The project is divided into two main phases:

1. **Command-Line Interface (CLI)**: 
   - Implement basic functions to list data (metadata, image list), add, delete, and extract images.
   - Provide options to extract images in different resolutions: "original", "small", or "thumbnail".

2. **Network Integration**:
   - Develop a web server to distribute images over the network using the HTTP protocol.
   - Extend CLI functionalities to be accessible via web interfaces.

### Implementation Details

#### File Format

The mgFS file format consists of three main parts:

1. **Header**: A fixed-size section containing system configuration details, created during the file's initialization.
2. **Metadata Array**: An array sized according to the max_files field in the header, storing metadata for each image, including their file positions.
3. **Image Data**: Images are stored sequentially in contiguous parts of the file.

#### Tools

Two main tools were developed for mgFS:

1. **Command-Line Interface**:
   - Allows users to list, read, add, and delete images from the mgFS file.
   
2. **Web Server**:
   - Provides the same functionalities as the CLI but accessible over the network using HTTP.
   
### Contributions

Our project was divided into two equal parts: operating system-level development and network integration. Each part was meticulously planned and executed to ensure seamless integration and high performance.

### What We Did

We completed all the assigned tasks, adhering closely to the provided descriptions and requirements. The project was divided into two main sections: operating system functionalities and network integration, each contributing 50% to the overall project.

### Remarks About Our Project

We meticulously commented on every function to ensure the code is clear and understandable. Our codebase is designed to be maintainable and extendable, with comprehensive documentation provided for each module.

### What You Should Know

There are no specific prerequisites to understand our implementation beyond the provided instructions. Our approach closely followed the guidelines, ensuring a robust and efficient solution for image management within a file system.
