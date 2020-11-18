#include "RenderEngine.h"

void Renderer::AddImage(ImageFile& img)
{
	images_.emplace_back(move(img));
}


Renderer::Renderer()
{
	sAppName = "IFF reader";
}


const vector<uint32_t> Renderer::GetData(const size_t n) const
{
	vector<uint32_t> contents;
	const auto data = images_.at(n).Get();
	for (unsigned int y = 0; y < data->height(); ++y) {
		for (unsigned int x = 0; x < data->width(); ++x) {
			const auto px = data->color_at(x, y);
			contents.push_back(px);
		}
	}
	return contents;
}


const size_t Renderer::GetFilePosByAbspath(const fs::path path) const
{
	for (size_t i = 0; i < images_.size(); ++i) {
		if (images_.at(i).IsPath(path)) {
			return i;
		}
	}
	return images_.size();
}


void Renderer::DisplayImage()
{
	// Select among the images already established.
	const auto& image_file = images_.at(current_image);
	const auto& this_image = image_file.Get();

	if (!image_file.IsLoaded())
	{ // Show black screen while loading.
		Clear(olc::BLACK);
		return;
	}

	if (this_image->width() != ScreenWidth() || this_image->height() != ScreenHeight()) {
		SetScreenSize(this_image->width(), this_image->height());
	}

	// Write pixels
	for (unsigned int y = 0; y < this_image->height(); ++y) {
		for (unsigned int x = 0; x < this_image->width(); ++x) {
			auto px = this_image->color_at(x, y);
			Draw(x, y, olc::Pixel(px));
		}
	}
}


void Renderer::BreakNoValidIFF()
{
	break_no_valid_iff = true;
}


const bool Renderer::InvalidIFF() const
{
	return break_no_valid_iff;
}


// Called once at the start, so create things here
bool Renderer::OnUserCreate()
{
	DisplayImage();
	return true;
}


const bool Renderer::BackKeyReleased()
{
	return GetKey(olc::Key::LEFT).bReleased || GetKey(olc::Key::BACK).bReleased;
}


const bool Renderer::ForwardKeyReleased()
{
	return GetKey(olc::Key::RIGHT).bReleased || GetKey(olc::Key::SPACE).bReleased;
}


bool Renderer::OnUserUpdate(float fElapsedTime)
{
	if (break_no_valid_iff > 0) {
		return false;
	}
	const auto image_count = images_.size();

	// You can apply colour correction now.
	auto& this_image = images_.at(current_image);



	if (GetKey(olc::Key::O).bReleased && this_image.OffersOCSColourCorrection()) {
		const auto currently_enabled = this_image.UsingOCSColourCorrection();
		this_image.ApplyOCSColourCorrection(!currently_enabled);
		DisplayImage();
		return true;
	}

	if (images_.size() > 1) {
		const auto stored_image = current_image;
		if (BackKeyReleased()) {
			current_image = (current_image == 0) ?
				image_count - 1 :
				current_image - 1;
		}
		if (ForwardKeyReleased()) {
			current_image = (current_image == image_count - 1) ?
				0 :
				current_image + 1;
		}
		if (stored_image != current_image) {
			Clear(olc::BLACK);
			DisplayImage();
		}
	}

	const auto exit_key_pressed =
		GetKey(olc::Key::ESCAPE).bPressed ||
		GetKey(olc::Key::ENTER).bPressed ||
		GetKey(olc::Key::Q).bPressed ||
		GetKey(olc::Key::X).bPressed;

	return (!exit_key_pressed);	// Close viewer on keypress.
}


const bool Renderer::Viewable() const
{
	return images_.size() != 0 && 
		any_of(
			begin(images_), 
			end(images_), 
			[](const ImageFile& f) { 
				return f.IsLoaded(); 
			});
}


const bool Renderer::AddImages(const vector<fs::path>& paths)
{   // Add files to collection (=open them for viewing).
	bool file_added = false;
	for (auto& f : paths) {
		if (auto image = ImageFile(f.string()); image.Get()) {
			AddImage(image);
			file_added = true;
		}
	}
	return file_added;
}


const bool Renderer::AddImage(const fs::path& path)
{   // Add file to collection (=open for viewing).
	if (auto image = ImageFile(path.string()); image.Get()) {
		AddImage(image);
		return true;
	}
	return false;
}
