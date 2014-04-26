## General ##
* Calling glGenBuffers, glBufferData, glDeleteBuffers, during runtime causes stuttering on some GPUs.
If dynamic buffers are needed, consider creating the buffer during initialization, and calling bufferData
once to allocate memory needed. To update it, bind it and call bufferSubData with the new vertex data.
This eliminates stuttering on my AMD HD6850 GPU.

Need some nice way to enforce this usage.

## BufferObject ##
* Double buffered bufferobjects are cool - implement that. Useful for updating dynamic data.

## MeshBuffer ##
* Get rid of the awful pointer chasing when calling Model::draw and similiar functions!
Model::Draw -> getActiveRenderer -> ar->draw(*this) -> Renderer::draw(Model &model) -> model.mesh->draw() -> MeshBuffer::draw() -> getActiveShader() -> ... stuff