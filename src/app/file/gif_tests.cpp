/* Aseprite
 * Copyright (C) 2014  David Capello
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "tests/test.h"

#include "app/context.h"
#include "app/document.h"
#include "app/file/file.h"
#include "app/file/file_formats_manager.h"
#include "app/file/gif_options.h"
#include "raster/raster.h"
#include "she/scoped_handle.h"
#include "she/system.h"

using namespace app;

typedef base::UniquePtr<Document> DocumentPtr;

class GifFormat : public ::testing::Test {
public:
  GifFormat() : m_system(she::create_system()) {
    FileFormatsManager::instance().registerAllFormats();
  }

protected:
  app::Context m_context;
  she::ScopedHandle<she::System> m_system;
};

TEST_F(GifFormat, Dimensions)
{
  const char* fn = "test.gif";

  {
    DocumentPtr doc(Document::createBasicDocument(IMAGE_INDEXED, 31, 29, 14));
    Sprite* sprite = doc->getSprite();
    doc->setFilename(fn);
    sprite->setTransparentColor(3);

    LayerImage* layer = dynamic_cast<LayerImage*>(sprite->getFolder()->getFirstLayer());
    ASSERT_NE((LayerImage*)NULL, layer);

    Image* image = sprite->getStock()->getImage(layer->getCel(FrameNumber(0))->getImage());
    clear_image(image, doc->getSprite()->getTransparentColor());

    save_document(&m_context, doc);
  }

  {
    DocumentPtr doc(load_document(&m_context, fn));
    Sprite* sprite = doc->getSprite();

    EXPECT_EQ(31, sprite->getWidth());
    EXPECT_EQ(29, sprite->getHeight());
    EXPECT_EQ(3, sprite->getTransparentColor());
    // TODO instead of 256, this should be 16 as Gif files contains
    // palettes that are power of two.
    EXPECT_EQ(256, sprite->getPalette(FrameNumber(0))->size());
  }
}

TEST_F(GifFormat, OpaqueIndexed)
{
  const char* fn = "test.gif";

  {
    DocumentPtr doc(Document::createBasicDocument(IMAGE_INDEXED, 2, 2, 4));
    Sprite* sprite = doc->getSprite();
    doc->setFilename(fn);

    Palette* pal = sprite->getPalette(FrameNumber(0));
    pal->setEntry(0, rgb(255, 255, 255));
    pal->setEntry(1, rgb(255, 13, 254));
    pal->setEntry(2, rgb(129, 255, 32));
    pal->setEntry(3, rgb(0, 0, 255));

    LayerImage* layer = dynamic_cast<LayerImage*>(sprite->getFolder()->getFirstLayer());
    layer->setBackground(true);
    ASSERT_NE((LayerImage*)NULL, layer);

    Image* image = sprite->getStock()->getImage(layer->getCel(FrameNumber(0))->getImage());
    image->putPixel(0, 0, 0);
    image->putPixel(0, 1, 1);
    image->putPixel(1, 0, 2);
    image->putPixel(1, 1, 3);

    save_document(&m_context, doc);
  }

  {
    DocumentPtr doc(load_document(&m_context, fn));
    Sprite* sprite = doc->getSprite();

    LayerImage* layer = dynamic_cast<LayerImage*>(sprite->getFolder()->getFirstLayer());
    ASSERT_NE((LayerImage*)NULL, layer);
    EXPECT_TRUE(layer->isBackground());

    Palette* pal = sprite->getPalette(FrameNumber(0));
    EXPECT_EQ(rgb(255, 255, 255), pal->getEntry(0));
    EXPECT_EQ(rgb(255, 13, 254), pal->getEntry(1));
    EXPECT_EQ(rgb(129, 255, 32), pal->getEntry(2));
    EXPECT_EQ(rgb(0, 0, 255), pal->getEntry(3));

    Image* image = sprite->getStock()->getImage(layer->getCel(FrameNumber(0))->getImage());
    EXPECT_EQ(0, sprite->getTransparentColor());
    EXPECT_EQ(0, image->getPixel(0, 0));
    EXPECT_EQ(1, image->getPixel(0, 1));
    EXPECT_EQ(2, image->getPixel(1, 0));
    EXPECT_EQ(3, image->getPixel(1, 1));
  }
}

TEST_F(GifFormat, TransparentIndexed)
{
  const char* fn = "test.gif";

  {
    DocumentPtr doc(Document::createBasicDocument(IMAGE_INDEXED, 2, 2, 4));
    Sprite* sprite = doc->getSprite();
    doc->setFilename(fn);

    Palette* pal = sprite->getPalette(FrameNumber(0));
    pal->setEntry(0, rgb(255, 255, 255));
    pal->setEntry(1, rgb(255, 13, 254));
    pal->setEntry(2, rgb(129, 255, 32));
    pal->setEntry(3, rgb(0, 0, 255));

    LayerImage* layer = dynamic_cast<LayerImage*>(sprite->getFolder()->getFirstLayer());
    ASSERT_NE((LayerImage*)NULL, layer);

    Image* image = sprite->getStock()->getImage(layer->getCel(FrameNumber(0))->getImage());
    image->putPixel(0, 0, 0);
    image->putPixel(0, 1, 1);
    image->putPixel(1, 0, 2);
    image->putPixel(1, 1, 3);

    save_document(&m_context, doc);
  }

  {
    DocumentPtr doc(load_document(&m_context, fn));
    Sprite* sprite = doc->getSprite();

    LayerImage* layer = dynamic_cast<LayerImage*>(sprite->getFolder()->getFirstLayer());
    ASSERT_NE((LayerImage*)NULL, layer);
    EXPECT_FALSE(layer->isBackground());

    Palette* pal = sprite->getPalette(FrameNumber(0));
    EXPECT_EQ(rgb(255, 255, 255), pal->getEntry(0));
    EXPECT_EQ(rgb(255, 13, 254), pal->getEntry(1));
    EXPECT_EQ(rgb(129, 255, 32), pal->getEntry(2));
    EXPECT_EQ(rgb(0, 0, 255), pal->getEntry(3));

    Image* image = sprite->getStock()->getImage(layer->getCel(FrameNumber(0))->getImage());
    EXPECT_EQ(0, sprite->getTransparentColor());
    EXPECT_EQ(0, image->getPixel(0, 0));
    EXPECT_EQ(1, image->getPixel(0, 1));
    EXPECT_EQ(2, image->getPixel(1, 0));
    EXPECT_EQ(3, image->getPixel(1, 1));
  }
}

TEST_F(GifFormat, TransparentRgbQuantization)
{
  const char* fn = "test.gif";

  {
    DocumentPtr doc(Document::createBasicDocument(IMAGE_RGB, 2, 2, 256));
    Sprite* sprite = doc->getSprite();
    doc->setFilename(fn);

    LayerImage* layer = dynamic_cast<LayerImage*>(sprite->getFolder()->getFirstLayer());
    ASSERT_NE((LayerImage*)NULL, layer);

    Image* image = sprite->getStock()->getImage(layer->getCel(FrameNumber(0))->getImage());
    image->putPixel(0, 0, rgba(0, 0, 0, 0));
    image->putPixel(0, 1, rgb(255, 0, 0));
    image->putPixel(1, 0, rgb(0, 255, 0));
    image->putPixel(1, 1, rgb(0, 0, 255));

    save_document(&m_context, doc);
  }

  {
    DocumentPtr doc(load_document(&m_context, fn));
    Sprite* sprite = doc->getSprite();

    LayerImage* layer = dynamic_cast<LayerImage*>(sprite->getFolder()->getFirstLayer());
    ASSERT_NE((LayerImage*)NULL, layer);

    Palette* pal = sprite->getPalette(FrameNumber(0));
    Image* image = sprite->getStock()->getImage(layer->getCel(FrameNumber(0))->getImage());
    EXPECT_EQ(0, sprite->getTransparentColor());
    EXPECT_EQ(0, image->getPixel(0, 0));
    EXPECT_EQ(rgb(255, 0, 0), pal->getEntry(image->getPixel(0, 1)));
    EXPECT_EQ(rgb(0, 255, 0), pal->getEntry(image->getPixel(1, 0)));
    EXPECT_EQ(rgb(0, 0, 255), pal->getEntry(image->getPixel(1, 1)));
  }
}

TEST_F(GifFormat, OpaqueRgbQuantization)
{
  const char* fn = "test.gif";

  {
    DocumentPtr doc(Document::createBasicDocument(IMAGE_RGB, 2, 2, 256));
    Sprite* sprite = doc->getSprite();
    doc->setFilename(fn);

    LayerImage* layer = dynamic_cast<LayerImage*>(sprite->getFolder()->getFirstLayer());
    layer->setBackground(true);
    ASSERT_NE((LayerImage*)NULL, layer);
    EXPECT_NE((LayerImage*)NULL, sprite->getBackgroundLayer());

    Image* image = sprite->getStock()->getImage(layer->getCel(FrameNumber(0))->getImage());
    image->putPixel(0, 0, rgb(0, 0, 0));
    image->putPixel(0, 1, rgb(255, 0, 0));
    image->putPixel(1, 0, rgb(0, 255, 0));
    image->putPixel(1, 1, rgb(0, 0, 255));

    save_document(&m_context, doc);
  }

  {
    DocumentPtr doc(load_document(&m_context, fn));
    Sprite* sprite = doc->getSprite();

    LayerImage* layer = dynamic_cast<LayerImage*>(sprite->getFolder()->getFirstLayer());
    ASSERT_NE((LayerImage*)NULL, layer);
    EXPECT_TRUE(layer->isBackground());
    EXPECT_EQ(layer, sprite->getBackgroundLayer());

    Palette* pal = sprite->getPalette(FrameNumber(0));
    Image* image = sprite->getStock()->getImage(layer->getCel(FrameNumber(0))->getImage());
    EXPECT_EQ(0, sprite->getTransparentColor());
    EXPECT_EQ(rgb(0, 0, 0), pal->getEntry(image->getPixel(0, 0)));
    EXPECT_EQ(rgb(255, 0, 0), pal->getEntry(image->getPixel(0, 1)));
    EXPECT_EQ(rgb(0, 255, 0), pal->getEntry(image->getPixel(1, 0)));
    EXPECT_EQ(rgb(0, 0, 255), pal->getEntry(image->getPixel(1, 1)));
  }
}

TEST_F(GifFormat, OpaqueRgbQuantizationTwoLayers)
{
  const char* fn = "test.gif";

  {
    DocumentPtr doc(Document::createBasicDocument(IMAGE_RGB, 2, 2, 256));
    Sprite* sprite = doc->getSprite();
    doc->setFilename(fn);

    LayerImage* layer1 = dynamic_cast<LayerImage*>(sprite->getFolder()->getFirstLayer());
    layer1->setBackground(true);

    LayerImage* layer2 = new LayerImage(sprite);
    sprite->getFolder()->addLayer(layer2);

    Image* image1 = sprite->getStock()->getImage(layer1->getCel(FrameNumber(0))->getImage());
    Image* image2 = Image::create(IMAGE_RGB, 2, 2);
    int image2Idx = sprite->getStock()->addImage(image2);
    Cel* cel2 = new Cel(FrameNumber(0), image2Idx);
    layer2->addCel(cel2);

    image1->clear(rgba(255, 255, 255, 255));
    image2->putPixel(0, 0, rgba(255, 0, 0, 255));
    image2->putPixel(1, 1, rgba(196, 0, 0, 255));

    Palette* pal = sprite->getPalette(FrameNumber(0));
    pal->setEntry(0, rgba(255, 255, 255, 255));
    pal->setEntry(1, rgba(255, 0, 0, 255));

    // Do not modify palettes
    doc->setFormatOptions(SharedPtr<FormatOptions>(new GifOptions(GifOptions::NoQuantize)));
    save_document(&m_context, doc);
  }

  {
    DocumentPtr doc(load_document(&m_context, fn));
    Sprite* sprite = doc->getSprite();

    LayerImage* layer = dynamic_cast<LayerImage*>(sprite->getFolder()->getFirstLayer());
    ASSERT_NE((LayerImage*)NULL, layer);
    ASSERT_TRUE(layer->isBackground());

    Palette* pal = sprite->getPalette(FrameNumber(0));
    Image* image = sprite->getStock()->getImage(layer->getCel(FrameNumber(0))->getImage());
    EXPECT_EQ(0, sprite->getTransparentColor());

    EXPECT_EQ(1, image->getPixel(0, 0));
    EXPECT_EQ(0, image->getPixel(0, 1));
    EXPECT_EQ(0, image->getPixel(1, 0));
    EXPECT_EQ(1, image->getPixel(1, 1));

    EXPECT_EQ(rgba(255, 255, 255, 255), pal->getEntry(0));
    EXPECT_EQ(rgba(255, 0, 0, 255), pal->getEntry(1));
  }
}
