#pragma once

#include "buffer.h"
#include "gl.h"

struct Vertex {
    
    glm::vec2 position;
    glm::vec2 texCoord;

    static void enable_attributes() {

        GL_CALL( glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, position)) );
        GL_CALL( glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, texCoord)) );

        GL_CALL( glEnableVertexAttribArray(0) );
        GL_CALL( glEnableVertexAttribArray(1) );
    }  
};

template<typename VertexType>
class VertexArray {

public:
    VertexArray()
        : _vertexBuffer(Buffer(GL_ARRAY_BUFFER)), _indexBuffer(Buffer(GL_ELEMENT_ARRAY_BUFFER))
    {

        GL_CALL( glCreateVertexArrays(1, &_id) );
        bind();
        _vertexBuffer.bind();
        _indexBuffer.bind();

        VertexType::enable_attributes();
    }

    ~VertexArray() {

        GL_CALL( glDeleteVertexArrays(1, &_id) );
    }

    void bind() const {

        GL_CALL( glBindVertexArray(_id) );
    }

    void vertex_data(const VertexType *data, std::size_t count) const {

        _vertexBuffer.bind();
        _vertexBuffer.push(data, count);
    }

    void index_data(const uint32_t *data, std::size_t count) {

        _indexBuffer.bind();
        _indexCount = count;
        _indexBuffer.push(data, count);
    }

    void draw(GLenum mode) const {

        bind();
        GL_CALL( glDrawElements(mode, _indexCount, GL_UNSIGNED_INT, NULL) );
    } 

private:
    GLuint _id;
    Buffer _vertexBuffer;
    Buffer _indexBuffer;
    std::size_t _indexCount;
};