#define TINYOBJLOADER_IMPLEMENTATION 

#include "Mesh.h"


Mesh::Mesh(const std::string_view& modelPath) : PATH(modelPath), modelMatrix(QMatrix4x4()), position(QVector3D(0.0f, 0.0f, 0.0f))
{
	modelMatrix.translate(position); 	
	loadModel(); 
}

void Mesh::setPosition(float x, float y, float z)
{
	auto newPos = QVector3D(x, y, z);
	modelMatrix.translate(newPos - position);
	position = newPos;
	
}

void Mesh::setRotation(float x, float y, float z)
{
	modelMatrix.translate(-position);
	modelMatrix.rotate(x, 1, 0, 0);
	modelMatrix.rotate(y, 0, 1, 0);
	modelMatrix.rotate(z, 0, 0, 1);
	modelMatrix.translate(position);
}


void Mesh::loadModel()
{
	tinyobj::ObjReaderConfig readerConfig; 	readerConfig.triangulate = true; 	tinyobj::ObjReader reader{}; 	if (!reader.ParseFromFile(PATH, readerConfig)) 		throw InvalidModelException("Failed to load file!");

	auto& attrib = reader.GetAttrib(); 	
	auto& shapes = reader.GetShapes(); 	
	std::array<Vertex, 3> faceVertices{};
		for (const auto& shape : shapes) {
		int indexOffset{}; 				
		for (int i = 0; i < shape.mesh.num_face_vertices.size(); i++) {
			int faceVerticesNum = shape.mesh.num_face_vertices.at(i); 						
			for (int j = 0; j < faceVerticesNum; j++) {
				auto index = shape.mesh.indices.at(indexOffset + j);
				faceVertices.at(j).position.setX(attrib.vertices[3 * index.vertex_index + 0]);
				faceVertices.at(j).position.setY(attrib.vertices[3 * index.vertex_index + 1]);
				faceVertices.at(j).position.setZ(attrib.vertices[3 * index.vertex_index + 2]);
				if (index.texcoord_index < 0) throw InvalidModelException("Missing texture coordinates");
				faceVertices.at(j).texCoords.setX(attrib.texcoords[2 * index.texcoord_index + 0]);
				faceVertices.at(j).texCoords.setY(1.0f - attrib.texcoords[2 * index.texcoord_index + 1]); 				
				if (index.normal_index < 0) throw InvalidModelException("Missing normals");
				faceVertices.at(j).normal.setX(attrib.normals[3 * index.normal_index + 0]);
				faceVertices.at(j).normal.setY(attrib.normals[3 * index.normal_index + 1]);
				faceVertices.at(j).normal.setZ(attrib.normals[3 * index.normal_index + 2]);
			}

			QVector3D tangent{}, bitangent{}, edge1 = faceVertices.at(1).position - faceVertices.at(0).position,
				edge2 = faceVertices.at(2).position - faceVertices.at(0).position;
			QVector2D deltaUV1 = faceVertices.at(1).texCoords - faceVertices.at(0).texCoords, deltaUV2 = faceVertices.at(2).texCoords - faceVertices.at(0).texCoords;
			float factor = 1.0f / (deltaUV1.x() * deltaUV2.y() - deltaUV1.y() * deltaUV2.x());
			tangent.setX((factor * (deltaUV2.y() * edge1.x() - deltaUV1.y() * edge2.x())));
			tangent.setY((factor * (deltaUV2.y() * edge1.y() - deltaUV1.y() * edge2.y())));
			tangent.setZ((factor * (deltaUV2.y() * edge1.z() - deltaUV1.y() * edge2.z())));
			tangent.normalize(); 						
			bitangent.setX((factor * (deltaUV1.x() * edge2.x() - deltaUV2.x() * edge1.x())));
			bitangent.setY((factor * (deltaUV1.x() * edge2.y() - deltaUV2.x() * edge1.y())));
			bitangent.setZ((factor * (deltaUV1.x() * edge2.z() - deltaUV2.x() * edge1.z())));
			bitangent.normalize(); 						
			for (auto& vertex : faceVertices) {
				vertices.emplace_back(vertex.position.x());
				vertices.emplace_back(vertex.position.y());
				vertices.emplace_back(vertex.position.z());
				vertices.emplace_back(vertex.texCoords.x());
				vertices.emplace_back(vertex.texCoords.y());
				vertices.emplace_back(vertex.normal.x());
				vertices.emplace_back(vertex.normal.y());
				vertices.emplace_back(vertex.normal.z());
				vertices.emplace_back(tangent.x());
				vertices.emplace_back(tangent.y());
				vertices.emplace_back(tangent.z());
				vertices.emplace_back(bitangent.x());
				vertices.emplace_back(bitangent.y());
				vertices.emplace_back(bitangent.z());
			}
			indexOffset += faceVerticesNum;
		}
	}
}
