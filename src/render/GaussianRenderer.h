#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include <glm/mat4x4.hpp>

#include "core/ShaderProgram.h"
#include "scene/GaussianModel.h"

namespace gs
{

	// ��˹��Ⱦ��������GPU��Դ�������������˹��Ԫ����
	class GaussianRenderer
	{
	public:
		GaussianRenderer() = default;
		~GaussianRenderer();

		// ��ֹ����������OpenGL��Դ�ظ��ͷ�
		GaussianRenderer(const GaussianRenderer&) = delete;
		GaussianRenderer& operator=(const GaussianRenderer&) = delete;

		// ��ʼ����ɫ����GPU������Դ
		bool initialize();
		// �ϴ�ģ�����ݵ�GPU
		bool uploadModel(const GaussianModel& model);
		// ִ��һ֡��Ⱦ
		void render(const glm::mat4& view, const glm::mat4& projection, float viewportWidth, float viewportHeight);
		// �����Ƿ����ø������Ը�˹
		void setUseAnisotropic(bool enabled);
		// ��ѯ�������Կ���
		bool useAnisotropic() const noexcept;
		// ����SH�������ᰴģ�������Զ��ü���
		int setShDegree(int degree);
		// ��ȡ��ǰSH����
		int shDegree() const noexcept;
		// ��ȡģ��֧�ֵ����SH����
		int maxSupportedShDegree() const noexcept;

	private:
		// GPU���Ԫ���֣�����shader�е�std430�ṹ�ϸ�һ�£�
		struct GPUSplat
		{
			float px;      // λ��x
			float py;      // λ��y
			float pz;      // λ��z
			float opacity; // ��͸����

			float sx;    // �߶�x
			float sy;    // �߶�y
			float sz;    // �߶�z
			float _pad0; // �������

			float rx; // ��ת��Ԫ��x
			float ry; // ��ת��Ԫ��y
			float rz; // ��ת��Ԫ��z
			float rw; // ��ת��Ԫ��w

			float cr;     // ��ɫr
			float cg;     // ��ɫg
			float cb;     // ��ɫb
			float radius; // �뾶����ֵ

			std::uint32_t shPacked[24]; // ������SHϵ����half2x16��
		};

		static_assert(sizeof(GPUSplat) == 160, "GPUSplat must match std430 array stride");

		// ���㲻С��value����С2����
		static std::size_t nextPow2(std::size_t value);
		// ȷ�������ۻ�Ŀ��ߴ�����Դ��Ч
		bool ensureAccumulationTarget(int width, int height);
		// ִ����ȼ�������GPU����
		void runDepthAndSort(const glm::mat4& view);
		// ִ�и�˹�ۻ����� pass
		void drawGaussianPass(const glm::mat4& view, const glm::mat4& projection, float viewportWidth, float viewportHeight, bool useReferencePath);
		// ִ���ۻ�����ϳ� pass
		void compositeAccumulationPass(GLint prevDrawFbo, GLint prevReadFbo, const std::array<GLint, 4>& prevViewport);

		ShaderProgram m_drawProgram;      // �����Ƴ���
		ShaderProgram m_depthProgram;     // ��ȼ��������
		ShaderProgram m_sortProgram;      // bitonic�������
		ShaderProgram m_compositeProgram; // �ϳɳ���

		unsigned int m_vao{ 0 };          // ������VAO
		unsigned int m_splatBuffer{ 0 };  // ��ԪSSBO
		unsigned int m_keysBuffer{ 0 };   // �����SSBO
		unsigned int m_indicesBuffer{ 0 };// ��������SSBO
		unsigned int m_accumFbo{ 0 };     // �ۻ�FBO
		unsigned int m_accumColorTex{ 0 };// �ۻ���ɫ����

		int m_accumWidth{ 0 };  // �ۻ���������
		int m_accumHeight{ 0 }; // �ۻ������߶�

		std::size_t m_splatCount{ 0 }; // ��Ч��Ԫ����
		std::size_t m_sortCount{ 0 };  // ����������2����

		GLint m_drawViewLoc{ -1 };         // ������u_viewλ��
		GLint m_drawProjLoc{ -1 };         // ������u_projλ��
		GLint m_drawViewportSizeLoc{ -1 }; // ������u_viewportSizeλ��
		GLint m_drawMaxPointSizeLoc{ -1 }; // ������u_maxPointSizeλ��
		GLint m_drawUseAnisotropicLoc{ -1 }; // ������u_useAnisotropicλ��
		GLint m_drawCameraPosLoc{ -1 };    // ������u_cameraPosλ��
		GLint m_drawShDegreeLoc{ -1 };     // ������u_shDegreeλ��

		GLint m_depthViewLoc{ -1 };         // ��ȳ���u_viewλ��
		GLint m_depthRealCountLoc{ -1 };    // ��ȳ���u_realCountλ��
		GLint m_depthSortCountLoc{ -1 };    // ��ȳ���u_sortCountλ��

		GLint m_sortCountLoc{ -1 };  // �������u_countλ��
		GLint m_sortStageLoc{ -1 };  // �������u_stageλ��
		GLint m_sortPassLoc{ -1 };   // �������u_passλ��
		GLint m_compositeTexLoc{ -1 };// �ϳɳ���u_accumTexλ��

		float m_maxPointSize{ 128.0f }; // �豸֧�ֵ�����ߴ�
		bool m_useAnisotropic{ true };  // �������Կ���
		int m_shDegree{ 1 };            // ��ǰSH����
		int m_maxSupportedShDegree{ 0 };// ģ��֧�ֵ����SH����
	};

} // namespace gs
